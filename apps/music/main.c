#include "ftk.h"
#include "ftk_app_music.h"

#ifdef HAS_MAIN
int main(int argc, char* argv[])
#else
Ret ftk_main(int argc, char* argv[])
#endif
{
	FtkApp* app = ftk_app_music_create();

#ifdef HAS_MAIN
	ftk_init(argc, argv);
#endif

	ftk_app_run(app, argc, argv);

#ifdef HAS_MAIN
	ftk_run();
#endif

	return RET_OK;
}

