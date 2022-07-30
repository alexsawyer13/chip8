#include <stdio.h>

struct args
{
    const char *path;
    const char *out;
};

int assemble(struct args *args);

int main(int argc, char *argv[])
{
    struct args args = {0};
    if (argc == 2)
    {
        args.path = argv[1];
        args.out = "a.ch8";
    }
    else if (argc == 3)
    {
        args.path = argv[1];
        args.out = argv[2];
    }
    else
    {
        printf("Usage: c8a <indir> <outdir:optional>\n");
        return 1;
    }

    printf("Assembling \"%s\" into \"%s\"\n", args.path, args.out);
    return assemble(&args);
}

int assemble(struct args *args)
{
    printf("Assembling currently not implemented\n");
    return 1;
}