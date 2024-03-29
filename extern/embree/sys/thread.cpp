// ======================================================================== //
// Copyright 2009-2012 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "thread.h"
#include "sysinfo.h"
#include "stl/string.h"

#include <iostream>
#include <xmmintrin.h>

#if defined(PTHREADS_WIN32)
#pragma comment (lib, "pthreadVC.lib")
#endif

////////////////////////////////////////////////////////////////////////////////
/// Windows Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace embree
{
  /*! set the affinity of a given thread */
  void setAffinity(HANDLE thread, ssize_t affinity)
  {
#if (_WIN32_WINNT >= 0x0601)
    int groups = GetActiveProcessorGroupCount();
    int totalProcessors = 0, group = 0, number = 0;
    for (int i = 0; i<groups; i++) {
      int processors = GetActiveProcessorCount(i);
      if (totalProcessors + processors > affinity) {
        group = i;
        number = (int)affinity - totalProcessors;
        break;
      }
      totalProcessors += processors;
    }

    GROUP_AFFINITY groupAffinity;
    groupAffinity.Group = (WORD)group;
    groupAffinity.Mask = (KAFFINITY)(uint64(1) << number);
    groupAffinity.Reserved[0] = 0;
    groupAffinity.Reserved[1] = 0;
    groupAffinity.Reserved[2] = 0;
    if (!SetThreadGroupAffinity(thread, &groupAffinity, NULL))
      throw std::runtime_error("cannot set thread group affinity");

    PROCESSOR_NUMBER processorNumber;
    processorNumber.Group = group;
    processorNumber.Number = number;
    processorNumber.Reserved = 0;
    if (!SetThreadIdealProcessorEx(thread, &processorNumber, NULL))
      throw std::runtime_error("cannot set thread ideal processor");
#else
    if (!SetThreadAffinityMask(thread, DWORD_PTR(uint64(1) << affinity)))
      throw std::runtime_error("cannot set thread affinity mask");
    if (SetThreadIdealProcessor(thread, (DWORD)affinity) == (DWORD)-1)
      throw std::runtime_error("cannot set thread ideal processor");
#endif
  }

  /*! set affinity of the calling thread */
  void setAffinity(ssize_t affinity) {
    setAffinity(GetCurrentThread(), affinity);
  }

  struct ThreadStartupData 
  {
  public:
    ThreadStartupData (thread_func f, void* arg) 
      : f(f), arg(arg) {}
  public:
    thread_func f;
    void* arg;
  };

  static void* threadStartup(ThreadStartupData* parg)
  {
    _mm_setcsr(_mm_getcsr() | /*FTZ:*/ (1<<15) | /*DAZ:*/ (1<<6));
    parg->f(parg->arg);
    delete parg;
    return NULL;
  }

#if !defined(PTHREADS_WIN32)

  /*! creates a hardware thread running on specific core */
  thread_t createThread(thread_func f, void* arg, size_t stack_size, ssize_t threadID)
  {
    HANDLE thread = CreateThread(NULL, stack_size, (LPTHREAD_START_ROUTINE)threadStartup, new ThreadStartupData(f,arg), 0, NULL);
    if (thread == NULL) throw std::runtime_error("cannot create thread");
    if (threadID >= 0) setAffinity(thread, threadID);
    return thread_t(thread);
  }

  /*! the thread calling this function gets yielded */
  void yield() {
    Sleep(0);
  }

  /*! waits until the given thread has terminated */
  void join(thread_t tid) {
    WaitForSingleObject(HANDLE(tid), INFINITE);
    CloseHandle(HANDLE(tid));
  }

  /*! destroy a hardware thread by its handle */
  void destroyThread(thread_t tid) {
    TerminateThread(HANDLE(tid),0);
    CloseHandle(HANDLE(tid));
  }

  /*! creates thread local storage */
  tls_t createTls() {
    return tls_t(TlsAlloc());
  }

  /*! set the thread local storage pointer */
  void setTls(tls_t tls, void* const ptr) {
    TlsSetValue(DWORD(size_t(tls)), ptr);
  }

  /*! return the thread local storage pointer */
  void* getTls(tls_t tls) {
    return TlsGetValue(DWORD(size_t(tls)));
  }

  /*! destroys thread local storage identifier */
  void destroyTls(tls_t tls) {
    TlsFree(DWORD(size_t(tls)));
  }
#endif
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// Linux Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX__)

namespace embree
{
  /*! set affinity of the calling thread */
  void setAffinity(ssize_t affinity)
  {
    cpu_set_t cset;
    CPU_ZERO(&cset);
    CPU_SET(affinity, &cset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cset), &cset) != 0)
      std::cerr << "Thread: cannot set affinity" << std::endl;
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// MacOSX Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__MACOSX__)

#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <mach/mach_init.h>

namespace embree
{
  /*! set affinity of the calling thread */
  void setAffinity(ssize_t affinity)
  {
    thread_affinity_policy ap;
    ap.affinity_tag = affinity;
    if (thread_policy_set(mach_thread_self(),THREAD_AFFINITY_POLICY,(integer_t*)&ap,THREAD_AFFINITY_POLICY_COUNT) != KERN_SUCCESS)
      std::cerr << "Thread: cannot set affinity" << std::endl;
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Unix Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__UNIX__) || defined(PTHREADS_WIN32)

#include <pthread.h>
#include <sched.h>

#if defined(__USE_NUMA__)
#include <numa.h>
#endif

namespace embree
{
  struct ThreadStartupData 
  {
  public:
    ThreadStartupData (thread_func f, void* arg, int affinity) 
      : f(f), arg(arg), affinity(affinity) {}
  public: 
    thread_func f;
    void* arg;
    ssize_t affinity;
  };

  static void* threadStartup(ThreadStartupData* parg)
  {
    _mm_setcsr(_mm_getcsr() | /*FTZ:*/ (1<<15) | /*DAZ:*/ (1<<6));

#if !defined(__LINUX__)
    if (parg->affinity >= 0)
      setAffinity(parg->affinity);
#endif

    parg->f(parg->arg);
    delete parg;
    return NULL;
  }

  /*! creates a hardware thread running on specific core */
  thread_t createThread(thread_func f, void* arg, size_t stack_size, ssize_t threadID)
  {
    /* set stack size */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size > 0) pthread_attr_setstacksize (&attr, stack_size);
    
    /* set affinity */
#if defined(__LINUX__)
    if (threadID >= 0) {
      cpu_set_t cset;
      CPU_ZERO(&cset);
      CPU_SET(threadID, &cset);
      pthread_attr_setaffinity_np(&attr,sizeof(cpu_set_t),&cset);
    }
#endif

    /* create thread */
    pthread_t* tid = new pthread_t;
    if (pthread_create(tid,&attr,(void*(*)(void*))threadStartup,new ThreadStartupData(f,arg,threadID)) != 0)
      throw std::runtime_error("pthread_create");

    return thread_t(tid);
  }

  /*! the thread calling this function gets yielded */
  void yield() {
    sched_yield();
  }

  /*! waits until the given thread has terminated */
  void join(thread_t tid) {
    if (pthread_join(*(pthread_t*)tid, NULL) != 0)
      throw std::runtime_error("pthread_join");
    delete (pthread_t*)tid;
  }

  /*! destroy a hardware thread by its handle */
  void destroyThread(thread_t tid) {
    pthread_cancel(*(pthread_t*)tid);
    delete (pthread_t*)tid;
  }

  /*! creates thread local storage */
  tls_t createTls() {
    pthread_key_t* key = new pthread_key_t;
    if (pthread_key_create(key,NULL) != 0)
      throw std::runtime_error("pthread_key_create");

    return tls_t(key);
  }

  /*! return the thread local storage pointer */
  void* getTls(tls_t tls) {
    return pthread_getspecific(*(pthread_key_t*)tls);
  }

  /*! set the thread local storage pointer */
  void setTls(tls_t tls, void* const ptr) {
    if (pthread_setspecific(*(pthread_key_t*)tls, ptr) != 0)
      throw std::runtime_error("pthread_setspecific");
  }

  /*! destroys thread local storage identifier */
  void destroyTls(tls_t tls) {
    if (pthread_key_delete(*(pthread_key_t*)tls) != 0)
      throw std::runtime_error("pthread_key_delete");
    delete (pthread_key_t*)tls;
  }
}

#endif
