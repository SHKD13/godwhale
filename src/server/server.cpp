#include "precomp.h"
#include "stdinc.h"

#include "server.h"
#include "client.h"

namespace godwhale {
namespace server {

shared_ptr<Server> Server::ms_instance;
mutex Server::ms_guard;

Server::Server()
    : m_isAlive(true), m_acceptor(m_service)
{
    m_acceptor.open(tcp::v4());
    m_acceptor.bind(tcp::endpoint(tcp::v4(), 4082));
    m_acceptor.listen(100);

    // IO関係の処理を開始します。
    m_thread.reset(new thread(&Server::ServiceThreadMain, shared_from_this()));
    BeginAccept();
}

Server::~Server()
{
    m_isAlive = false;

    // 例外が出ないよう、念のためerrorを使っています。
    system::error_code error;
    m_acceptor.cancel(error);
    m_acceptor.close(error);

    if (m_thread != NULL) {
        m_thread->join();
        m_thread.reset();
    }
}

void Server::ServiceThreadMain()
{
    while (m_isAlive) {
        try
        {
            system::error_code error;

            // 処理した量が０であれば、少しウェイトを入れます。
            auto count = m_service.run(error);
            if (count == 0) {
                this_thread::sleep(posix_time::milliseconds(1));
            }

            m_service.reset();
        }
        catch (...) {
            LOG(Error) << "ServerのIOスレッドでエラーが発生しました。";
        }
    }
}

void Server::ClientDisconnected(shared_ptr<Client> client)
{
    if (client == NULL) return;

    ScopedLock lock(m_guard);
    m_clientList.remove(client);
}

void Server::BeginAccept()
{
    if (!m_isAlive) return;

    try
    {
        shared_ptr<tcp::socket> socket(new tcp::socket(m_service));

        m_acceptor.async_accept(*socket,
            bind(&Server::HandleAccept, shared_from_this(),
                 socket, asio::placeholders::error));
    }
    catch (std::exception ex)
    {
        LOG(Error) << "acceptの開始処理に失敗しました。(" << ex.what() << ")";
        m_isAlive = false;
    }
    catch (...)
    {
        LOG(Error) << "acceptの開始処理に失敗しました。";
        m_isAlive = false;
    }
}

void Server::HandleAccept(shared_ptr<tcp::socket> socket,
                          const system::error_code &error)
{
    if (error) {
        LOG(Error) << "acceptに失敗しました。(" << error.message() << ")";
    }
    else {
        shared_ptr<Client> client(new Client(this, socket));

        // クライアントをリストに追加します。
        {
            ScopedLock lock(m_guard);
            m_clientList.push_back(client);
        }

        client->BeginAsyncReceive();
        LOG(Notification) << "クライアントを受理しました。";
    }

    BeginAccept();
}

int Server::Iterate(int *value, std::vector<move_t> &pvseq)
{
    *value = 0;
    return 0;
}

} // namespace server
} // namespace godwhale
