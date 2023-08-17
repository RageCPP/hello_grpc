#include <absl/flags/internal/flag.h>
#include <cstdint>
#include <grpcpp/completion_queue.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/status.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "people.grpc.pb.h"
#include "people.pb.h"

/**
 *  TODO:
 *  - [ ] 并发处理请求 https://zhuanlan.zhihu.com/p/420656059
 *  - [ ] 对于未注册的 rpc 的处理
 */

ABSL_FLAG(uint16_t, port, 50051, "server port for the service");

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using student::AddStudentRequest;
using student::Empty;
using student::ManagementSystem;
using student::SearchAgeReply;
using student::SearchAgeRequest;

class ServerImpl final
{
  public:
    ~ServerImpl()
    {
        std::cout << "服务器关闭" << std::endl;
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run(uint16_t port)
    {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;
        HandleRPCS();
    }

  private:
    class CallDataBase
    {
      protected:
        virtual void CreateAction() = 0;
        virtual void ProcessAction() = 0;
        enum CallStatus
        {
            CREATE,
            PROCESS,
            FINISH
        };

      public:
        virtual void Proceed() = 0;
        CallDataBase(){};
        virtual ~CallDataBase() = default;
    };
    template <class RequestType, class ReplyType> class CallData : CallDataBase
    {
      public:
        CallData(ManagementSystem::AsyncService *service, ServerCompletionQueue *cq)
            : status_{CREATE}, service_{service}, cq_{cq}, responder_{&ctx_}
        {
        }
        virtual void AddNextToCompletionQueue() = 0;
        virtual void Proceed() override
        {
            switch (status_)
            {
            case CREATE:
                status_ = PROCESS;
                CreateAction();
                break;
            case PROCESS:
                AddNextToCompletionQueue();
                ProcessAction();
                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
                break;
            case FINISH:
                GPR_ASSERT(status_ == FINISH);
                delete this;
                break;
            }
        }

      protected:
        ManagementSystem::AsyncService *service_;
        CallStatus status_;
        RequestType request_;
        ReplyType reply_;
        ServerAsyncResponseWriter<ReplyType> responder_;
        ServerContext ctx_;
        ServerCompletionQueue *cq_;
    };
    class SearchAge : CallData<SearchAgeRequest, SearchAgeReply>
    {
      public:
        SearchAge(ManagementSystem::AsyncService *service, ServerCompletionQueue *cq) : CallData(service, cq)
        {
            Proceed();
        }

      protected:
        virtual void AddNextToCompletionQueue() override
        {
            new SearchAge(service_, cq_);
        }
        virtual void CreateAction() override
        {
            std::cout << "search age" << std::endl;
            service_->RequestSearchAge(&ctx_, &request_, &responder_, cq_, cq_, this);
        }
        virtual void ProcessAction() override
        {
            reply_.set_age(10);
        }
    };
    class AddStudent : CallData<AddStudentRequest, Empty>
    {
      public:
        AddStudent(ManagementSystem::AsyncService *service, ServerCompletionQueue *cq) : CallData(service, cq)
        {
            Proceed();
        }

      protected:
        virtual void AddNextToCompletionQueue() override
        {
            new AddStudent(service_, cq_);
        }
        virtual void CreateAction() override
        {
            std::cout << "add student" << std::endl;
            service_->RequestAddStudent(&ctx_, &request_, &responder_, cq_, cq_, this);
        }
        virtual void ProcessAction() override
        {
        }
    };

    void HandleRPCS()
    {
        new SearchAge(&service_, cq_.get());
        new AddStudent(&service_, cq_.get());
        void *tag;
        bool ok;
        while (true)
        {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<CallDataBase *>(tag)->Proceed();
        }
    };
    std::unique_ptr<ServerCompletionQueue> cq_;
    std::unique_ptr<Server> server_;
    ManagementSystem::AsyncService service_;
};

int main(int argc, char **argv)
{
    absl::ParseCommandLine(argc, argv);
    ServerImpl server;
    server.Run(absl::GetFlag(FLAGS_port));
    return 0;
}
