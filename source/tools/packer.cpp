/*////////////////////////////////////////////////////////////////////////////*/

#define NK_NPAK_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION

#define NK_STATIC

#include <stdio.h>

#include <nk_npak.h>

int main(int argc, char** argv)
{
    printf("Packing game assets into npak... ");
    nkBool res = nk_npak_pack("binary/win32/assets.npak", "assets");
    printf("%s!\n\n", res ? "successful" : "failure");
    return 0;
}

/*////////////////////////////////////////////////////////////////////////////*/
