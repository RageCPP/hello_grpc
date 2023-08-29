// #include "absl/flags/flag.h"
// #include "absl/flags/parse.h"
#include "people.grpc.pb.h"
#include "people.pb.h"
// #include <absl/flags/internal/flag.h>
#include <cstdint>
#include <grpc/status.h>
#include <grpc/support/log.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/status.h>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using student::AddStudentRequest;
using student::Empty;
using student::ManagementSystem;
using student::SearchAgeReply;
using student::SearchAgeRequest;

// ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

struct Student
{
    std::string name;
    uint8_t age;
};

class Cli
{
  public:
    Cli(std::shared_ptr<Channel> channel) : stub_{ManagementSystem::NewStub(channel)}
    {
    }
    Status AddStudent(const Student &student)
    {
        AddStudentRequest request;
        request.set_age(student.age);
        request.set_name(student.name);
        Empty reply;
        ClientContext context;
        Status status = stub_->AddStudent(&context, request, &reply);
        if (!status.ok())
        {
            std::cout << "add student reply error" << std::endl;
        }
        return status;
    }
    Status SearchAge(const std::string &name, int32_t &age)
    {
        SearchAgeRequest request;
        request.set_name(name);
        SearchAgeReply reply;
        ClientContext context;
        Status status = stub_->SearchAge(&context, request, &reply);
        if (status.ok())
        {
            age = reply.age();
        }
        if (!status.ok())
        {
            std::cout << "search student age reply error" << std::endl;
        }
        return status;
    }

  private:
    std::unique_ptr<ManagementSystem::Stub> stub_;
};

int main(int argc, char **argv)
{
    // absl::ParseCommandLine(argc, argv);
    // std::string target_str = absl::GetFlag(FLAGS_target);
    Cli cli{grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())};
    auto age = 0;
    auto reply = cli.SearchAge("王", age);
    GPR_ASSERT(reply.ok());
    std::cout << age << std::endl;
    reply = cli.AddStudent(Student{.name = "王", .age = 26});
    GPR_ASSERT(reply.ok());
    return 0;
}
