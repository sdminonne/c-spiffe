/**
 * This is a sample client that uses the spiffe::WorkloadAPIClient class
 * to fetch X.509 SVIDs
 */

#include <unistd.h>
#include <csignal>
#include "c-spiffe.h"

#define INITIAL_DELAY 1000000
#define TIMEOUT INITIAL_DELAY * 30
useconds_t delay = INITIAL_DELAY;

void updatedX509Callback(X509SVIDResponse x509SVIDResponse) {
    // Successfull response received. Reset delay.
    delay = INITIAL_DELAY;

    std::cout << "Fetched X509SVID with SPIFFE ID: " << x509SVIDResponse.svids(0).spiffe_id() << std::endl;
}

/**
 * Fetch SVIDs. If there is a failure, it retries with back-off.
 */
void fetchX509SVIDs() {
    spiffe::WorkloadAPIClient workloadClient("unix:/tmp/agent.sock", updatedX509Callback);

    Start:
    workloadClient.FetchX509SVIDs();
    if (workloadClient.GetFetchX509SVIDsStatus().ok()) {
        std::cout << "FetchX509SVID rpc succeeded." << std::endl;
    } else {
        std::cout << "FetchX509SVID rpc failed. Error code: " << workloadClient.GetFetchX509SVIDsStatus().error_code() << ". Error message: " << workloadClient.GetFetchX509SVIDsStatus().error_message() << std::endl;
        usleep(delay);
        if (delay < TIMEOUT) {
            delay += delay;
            goto Start;
        } else {
            throw std::runtime_error("Timeout");
        }
    }
}

int main(int argc, char** argv) {
    try {
        fetchX509SVIDs();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

	return 0;
}
