#include <spdlog/spdlog.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <stb_image.h>

void dependency_test()
{
    spdlog::info("Dependencies loaded!");
}

int main()
{
    dependency_test();
    return 0;
}