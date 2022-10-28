#include "Application.h"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[])
{ 
    qputenv("QT_FATAL_WARNINGS", "1");
    spdlog::set_pattern("[%H:%M:%S] %v");
    spdlog::set_automatic_registration(false);    

    Application kApp(argc,argv);
    return kApp.exec();
}
