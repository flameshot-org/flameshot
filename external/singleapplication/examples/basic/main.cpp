#include <singleapplication.h>

int main(int argc, char* argv[])
{
    // Allow secondary instances
    SingleApplication app(argc, argv);

    qWarning() << "Started a new instance";

    return app.exec();
}
