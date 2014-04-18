#include "precomp.h"
#include "stdinc.h"

#include "server.h"
#include "client.h"

namespace godwhale {
namespace server {

shared_ptr<Server> Server::ms_instance;

/**
 * �T�[�o�[�̃V���O���g���C���X�^���X�����������܂��B
 */
void Server::Initialize()
{
    ms_instance.reset(new Server());

    ms_instance->StartThread();
}

Server::Server()
    : m_isAlive(true), m_acceptor(m_service), m_gid(0)
{
    m_acceptor.open(tcp::v4());
    m_acceptor.bind(tcp::endpoint(tcp::v4(), 4082));
    m_acceptor.listen(100);
}

Server::~Server()
{
    m_isAlive = false;

    // ��O���o�Ȃ��悤�A�O�̂���error���g���Ă��܂��B
    system::error_code error;
    m_acceptor.cancel(error);
    m_acceptor.close(error);

    // foreach����m_clientList���ύX�����\�������邽�߁A
    // �O�̂��߈ꎞ�ϐ��ɃR�s�[���܂��B
    auto tmp = CloneClientList();
    BOOST_FOREACH(auto client, tmp) {
        client.reset();
    }

    {
        ScopedLock lock(m_guard);
        m_clientList.clear();
    }

    if (m_thread != NULL) {
        m_thread->join();
        m_thread.reset();
    }
}

/**
 * @brief IO�X���b�h���J�n���܂��B
 */
void Server::StartThread()
{
    // IO�֌W�̏������J�n���܂��B
    m_thread.reset(new thread(&Server::ServiceThreadMain, this));
    BeginAccept();
}

void Server::ServiceThreadMain()
{
    while (m_isAlive) {
        try
        {
            system::error_code error;

            // ���������ʂ��O�ł���΁A�����E�F�C�g�����܂��B
            auto count = m_service.run(error);
            if (count == 0) {
                this_thread::sleep(posix_time::milliseconds(1));
            }

            m_service.reset();
        }
        catch (...) {
            LOG(Error) << "Server��IO�X���b�h�ŃG���[���������܂����B";
        }
    }
}

/**
 * @brief �N���C�A���g�̃��X�g�����S�ɃR�s�[���܂��B
 */
std::list<shared_ptr<Client> > Server::CloneClientList()
{
    ScopedLock lock(m_guard);

    return m_clientList;
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
        LOG(Error) << "accept�̊J�n�����Ɏ��s���܂����B(" << ex.what() << ")";
        m_isAlive = false;
    }
    catch (...)
    {
        LOG(Error) << "accept�̊J�n�����Ɏ��s���܂����B";
        m_isAlive = false;
    }
}

void Server::HandleAccept(shared_ptr<tcp::socket> socket,
                          const system::error_code &error)
{
    if (error) {
        LOG(Error) << "accept�Ɏ��s���܂����B(" << error.message() << ")";
    }
    else {
        shared_ptr<Client> client(new Client(this, socket));

        // �N���C�A���g�����X�g�ɒǉ����܂��B
        {
            ScopedLock lock(m_guard);
            m_clientList.push_back(client);
        }

        client->BeginAsyncReceive();
        LOG(Notification) << "�N���C�A���g���󗝂��܂����B";
    }

    BeginAccept();
}

} // namespace server
} // namespace godwhale
