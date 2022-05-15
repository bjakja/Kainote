#pragma once

#if !defined(NDEBUG)
	#define DEBUG_LOG(log) std::cout << log << " (" __FILE__ ":" << __LINE__ << ")" << std::endl
#else
	#define DEBUG_LOG(x)
#endif
