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
    asio::socket_base::reuse_address option(true);
    m_acceptor.set_option(option);

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

struct Compare
{
    bool operator()(weak_ptr<Client> x, weak_ptr<Client> y) const
    {
        auto px = x.lock();
        auto py = y.lock();

        if (px == NULL) {
            return false;
        }
        else if (py == NULL) {
            return true;
        }
        else {
            return (px->GetThreadCount() > py->GetThreadCount());
        }
    }
};

/**
 * @brief �N���C�A���g�����O�C�������Ƃ��ɌĂ΂�܂��B
 */
void Server::ClientLogined(shared_ptr<Client> client)
{
    ScopedLock locker(m_guard);

    m_clientList.push_back(client);

    // ���\���Ƀ\�[�g���܂��B
    //std::stable_sort(m_clientList.begin(), m_clientList.end(), Compare());
    m_clientList.sort(Compare());
}

/**
 * @brief �s�v�ȃN���C�A���g���폜���A�����Ă���I�u�W�F�N�g�݂̂����o���܂��B
 */
std::vector<shared_ptr<Client> > Server::GetClientList()
{
    ScopedLock locker(m_guard);
    std::vector<shared_ptr<Client> > result;

    // ������Ƃ���������
    result.reserve(m_clientList.size());

    for (auto it = m_clientList.begin(); it != m_clientList.end(); ) {
        auto client = (*it).lock();

        if (client != NULL) {
            result.push_back(client);
            ++it;
        }
        else {
            it = m_clientList.erase(it);
        }
    }

    return result;
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
        shared_ptr<Client> client(new Client(shared_from_this(), socket));

        // �N���C�A���g�����X�g�ɒǉ����܂��B
        /*LOCK(m_guard) {
            m_clientList.push_back(client);
        }*/

        client->BeginAsyncReceive();
        LOG(Notification) << "�N���C�A���g���󗝂��܂����B";
    }

    BeginAccept();
}

} // namespace server
} // namespace godwhale
