/*========================================================

 XephTools - Timer
 Copyright (C) 2022 Jon Bogert (jonbogert@gmail.com)

 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it freely,
 subject to the following restrictions:

 1. The origin of this software must not be misrepresented;
	you must not claim that you wrote the original software.
	If you use this software in a product, an acknowledgment
	in the product documentation would be appreciated but is not required.

 2. Altered source versions must be plainly marked as such,
	and must not be misrepresented as being the original software.

 3. This notice may not be removed or altered from any source distribution.

========================================================*/

#ifndef XE_TIMER_H
#define XE_TIMER_H

#include <chrono>

namespace xe
{
	class Timer
	{
		std::chrono::high_resolution_clock::time_point m_startPoint;

	public:
		Timer()
		{
			Reset();
		}

		void Reset()
		{
			m_startPoint = std::chrono::high_resolution_clock::now();
		}

		float GetElapsed()
		{
			std::chrono::microseconds delta;
			delta = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_startPoint);
			return std::chrono::duration<float>(delta).count();
		}

		float DeltaTime()
		{
			float deltaTime = GetElapsed();
			Reset();
			return deltaTime;
		}

		float FPS()
		{
			float deltaTime = DeltaTime();
			if (deltaTime != 0.f)
				return 1.f / deltaTime;
			else
			{
				return 0.f;
			}
		}
	};
}
#endif // XE_TIMER_H