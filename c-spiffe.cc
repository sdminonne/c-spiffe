#include <grpc++/create_channel.h>
#include "workload.grpc.pb.h"
#include "c-spiffe.h"

using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

std::string const SECURITY_HEADER = "workload.spiffe.io";

spiffe::WorkloadAPIClient::WorkloadAPIClient(std::string socket_address, X509SVIDsUpdatedCallback updatedCallback) {
    if (socket_address == "") {
        if (std::getenv("SPIFFE_ENDPOINT_SOCKET") == NULL) {
            throw std::invalid_argument("SPIFFE endpoint socket not specified");
        }
        socket_address = std::string(std::getenv("SPIFFE_ENDPOINT_SOCKET"));
    }

    this->socket_address = socket_address;
    this->updatedCallback = updatedCallback;
}

void spiffe::WorkloadAPIClient::StopFetchingX509SVIDs() {
    stopFetchingX509SVIDs = true;
}

Status spiffe::WorkloadAPIClient::GetFetchX509SVIDsStatus() {
    return fetchX509SVIDsStatus;
}

void spiffe::WorkloadAPIClient::FetchX509SVIDs() {
    stopFetchingX509SVIDs = false;
    std::unique_ptr<SpiffeWorkloadAPI::Stub> stub;
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(socket_address, grpc::InsecureChannelCredentials());

    stub = SpiffeWorkloadAPI::NewStub(channel);

    X509SVIDRequest x509SVIDRequest;
    X509SVIDResponse x509SVIDResponse;
    ClientContext context;
    context.AddMetadata(SECURITY_HEADER, "true");

    std::unique_ptr<ClientReader<X509SVIDResponse>> reader(stub->FetchX509SVID(&context, x509SVIDRequest));

    while (reader->Read(&x509SVIDResponse) && !stopFetchingX509SVIDs) {
        updatedCallback(x509SVIDResponse);
    }

    fetchX509SVIDsStatus = reader->Finish();
}
