#include "includes.h"
#include "app.h"

int main() {
    
    if (!tb_init("data/syzygy/wdl;data/syzygy/dtz")) {
		std::cerr << "Error loading syzygy" << std::endl;
	}

    App app;
    app.run();
    
    return 0;
}