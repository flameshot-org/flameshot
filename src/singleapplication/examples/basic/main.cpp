#include <SingleApplication.h>

int main(int argc, char *argv[])
{
    // Allow secondary instances
    SingleApplication app( argc, argv );

    return app.exec();
}
