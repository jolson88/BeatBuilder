#pragma once
#include "ISoundSource.h"

namespace BeatBuilder {
	namespace Audio {
		public interface class ISoundListener
		{
			void ListenTo(ISoundSource^ source);
		};
	}
}

