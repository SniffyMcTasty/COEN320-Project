#include "common.h"
#include "Radar.h"
#include "Plane.h"

void testRadar();

int lolz(int argc, char *argv[]) {

    srand(time(NULL));
    cout << "***** APPLICATION START *****" << endl;

    testRadar();

    cout << "***** APPLICATION END *****" << endl;
    return EXIT_SUCCESS;
}

void testRadar() {
	const int N = 3;
    vector<PlaneInfo_t> infos;
    vector<Plane*> airspace;
    Radar radar(&airspace);

    for (int i = 0; i < N; i++)
    	infos.push_back(Plane::randomInfo());

    while (!infos.empty() /*|| !radar.noPlanes*/) {
    	delay(1000);

    	cout << "Radar Ping:" << endl;
    	radar.getPlanes();

    	if (!infos.empty()) {
    		static int cnt = 0;
    		if (cnt++ >= 2) {
    			cnt = 0;
    			airspace.push_back(new Plane(infos.back()));
    			infos.pop_back();
    		}
    	}
    }

//    radar.exit();

    for (Plane* plane : airspace) {
    	plane->join();
    	delete plane;
    }

    radar.join();
}
