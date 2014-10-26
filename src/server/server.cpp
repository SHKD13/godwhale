#include "precomp.h"
#include "stdinc.h"

#include "server.h"
#include "serverclient.h"
#include "commandpacket.h"

namespace godwhale {
namespace server {

using namespace boost;
typedef boost::asio::ip::tcp tcp;

shared_ptr<Server> Server::ms_instance;

/**
 * �T�[�o�[�̃V���O���g���C���X�^���X�����������܂��B
 */
void Server::initialize()
{
    ms_instance.reset(new Server());

    ms_instance->startThread();
}

Server::Server()
    : m_isAlive(true), m_acceptor(m_service), m_gid(0), m_isPlaying(false)
    , m_currentValue(0)
{
    m_acceptor.open(tcp::v4());
    asio::socket_base::reuse_address option(true);
    m_acceptor.set_option(option);

    LOG_NOTIFICATION() << "server port=" << 4082;

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
void Server::startThread()
{
    if (m_thread != nullptr) {
        return;
    }

    // IO�֌W�̏������J�n���܂��B
    m_thread.reset(new thread(&Server::serviceThreadMain, this));
    beginAccept();
}

/**
 * @brief IO�X���b�h�̃��C�����\�b�h�ł��B
 */
void Server::serviceThreadMain()
{
    while (m_isAlive) {
        try {
            system::error_code error;

            // ���������ʂ��O�ł���΁A�����E�F�C�g�����܂��B
            auto count = m_service.poll_one(error);
            if (count == 0) {
                this_thread::sleep(posix_time::milliseconds(20));
            }

            m_service.reset();
        }
        catch (...) {
            LOG_ERROR() << "Server��IO�X���b�h�ŃG���[���������܂����B";
        }
    }
}

void Server::beginAccept()
{
    if (!m_isAlive) return;

    try
    {
        shared_ptr<tcp::socket> socket(new tcp::socket(m_service));

        m_acceptor.async_accept(*socket,
            bind(&Server::handleAccept, shared_from_this(),
                 socket, asio::placeholders::error));
    }
    catch (std::exception ex)
    {
        LOG_ERROR() << "accept�̊J�n�����Ɏ��s���܂����B(" << ex.what() << ")";
        m_isAlive = false;
    }
    catch (...)
    {
        LOG_ERROR() << "accept�̊J�n�����Ɏ��s���܂����B";
        m_isAlive = false;
    }
}

void Server::handleAccept(shared_ptr<tcp::socket> socket,
                          const system::error_code &error)
{
    if (error) {
        LOG_ERROR() << "accept�Ɏ��s���܂����B(" << error.message() << ")";
    }
    else {
        shared_ptr<ServerClient> sclient(new ServerClient(shared_from_this()));
        sclient->initialize(socket);

        // �N���C�A���g�����X�g�ɒǉ����܂��B
        LOCK(m_guard) {
            m_clientList.push_back(sclient);
        }
        
        LOG_NOTIFICATION() << "�N���C�A���g��Accept���܂����B";
    }

    beginAccept();
}

struct Compare
{
    bool operator()(weak_ptr<ServerClient> x, weak_ptr<ServerClient> y) const
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
            return (px->getThreadCount() > py->getThreadCount());
        }
    }
};

/**
 * @brief �N���C�A���g�����O�C�������Ƃ��ɌĂ΂�܂��B
 */
void Server::clientLogined(shared_ptr<ServerClient> client)
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
std::vector<shared_ptr<ServerClient>> Server::getClientList()
{
    ScopedLock locker(m_guard);
    std::vector<shared_ptr<ServerClient>> result;

    // ������Ƃ���������
    result.reserve(m_clientList.size());

    for (auto it = m_clientList.begin(); it != m_clientList.end(); ) {
        auto client = *it;

        if (client != NULL && client->isAlive()) {
            result.push_back(client);
            ++it;
        }
        else {
            it = m_clientList.erase(it);
        }
    }

    return std::move(result);
}

} // namespace server
} // namespace godwhale
