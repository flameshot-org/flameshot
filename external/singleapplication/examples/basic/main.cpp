#include <singleapplication.h>

int main(int argc, char *argv[])
{
    SingleApplication app( argc, argv );

    qWarning() << "Started a new instance";

    return app.exec();
}
