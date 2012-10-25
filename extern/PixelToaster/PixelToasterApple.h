// Apple MacOS X Platform
// Copyright © 2004-2007 Glenn Fiedler
// Part of the PixelToaster Framebuffer Library - http://www.pixeltoaster.com

// native Cocoa output implemented by Thorsten Schaaps <bitpull@aixplosive.de>

#ifndef PIXELTOASTER_APPLE_USE_X11
	#define PIXELTOASTER_APPLE_USE_X11	0
#endif

#include "CoreServices/CoreServices.h"
#include <mach/mach_time.h>

#if PIXELTOASTER_APPLE_USE_X11
	#define PIXELTOASTER_NO_UNIX_TIMER
	#include "PixelToasterUnix.h"
#endif

// display implementation
namespace PixelToaster
{
	#if !PIXELTOASTER_APPLE_USE_X11		
		class AppleDisplay : public DisplayAdapter
		{
			class AppleDisplayPrivate;
			
		public:
			AppleDisplay();
			
			virtual ~AppleDisplay();
			
			virtual bool open( const char title[],
												 int width, int height,
												 Output output,
												 Mode mode );
			
			virtual void close();
			
			virtual bool update( const TrueColorPixel *			trueColorPixels,
													 const FloatingPointPixel * floatingPointPixels,
													 const Rectangle *					dirtyBox );
			
			virtual void title( const char title[] );
			
			virtual bool windowed();
			
			virtual bool fullscreen();

			virtual void listener( Listener * listener );
			
			void setShouldClose()  { _shouldClose = true; }

			void setShouldToggle() { _shouldToggle = true; }

			void shutdown();

		protected:
			
			virtual void defaults();

		private:

			AppleDisplayPrivate*	_private;
			bool									_shouldClose;
			bool									_shouldToggle;
		}; 
	#else
		class AppleDisplay : public UnixDisplay 
		{
			// ...
		}; 
	#endif

	// timer implementation
	class AppleTimer : public TimerInterface
	{
	public:
	
		AppleTimer()
		{
			mach_timebase_info_data_t timebaseInfo;
			(void) mach_timebase_info(&timebaseInfo);
			_conversion = 1e-9 * (double) timebaseInfo.numer / (double) timebaseInfo.denom;
			reset();
		}

		void reset()
		{
			_timeCounter = mach_absolute_time();
			_deltaCounter = _timeCounter;
			_time = 0;
		}
	
		double time()
		{
			UInt64 counter = mach_absolute_time();
			UInt64 delta = counter - _timeCounter;
			_timeCounter = counter;
			_time += delta * _conversion;
			return _time;
		}
	
		double delta()
		{
			UInt64 counter = mach_absolute_time();
			UInt64 delta = counter - _deltaCounter;
			_deltaCounter = counter;
			return delta * _conversion;
		}
	
		double resolution()
		{
			return _conversion;
		}
	
		void wait( double seconds )
		{
			UInt64 counter = mach_absolute_time();
			UInt64 finish = counter + UInt64( seconds/_conversion );
			while  ( counter < finish )
				counter = mach_absolute_time();
		}
	
	private:
	
		double _time;				///< current time in seconds
		UInt64 _timeCounter;		///< time counter in mach time
		UInt64 _deltaCounter;		///< delta counter in mach time
		double _conversion;			///< conversion factor to nanoseconds
	};
}
