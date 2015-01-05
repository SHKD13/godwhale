#include "precomp.h"
#include "stdinc.h"
#include "server.h"
#include "serverclient.h"
#include "movenodetree.h"
#include "commandpacket.h"

namespace godwhale {
namespace server {

typedef boost::asio::ip::tcp tcp;

shared_ptr<Server> Server::ms_instance;

/**
 * �T�[�o�[�̃V���O���g���C���X�^���X�����������܂��B
 */
void Server::initialize(int argc, char * argv[])
{
    ms_instance.reset(new Server());

    ms_instance->startThread();
}

Server::Server()
    : m_acceptor(m_service), m_isAlive(true), m_positionId(0)
    , m_ntree(new MoveNodeTree(1)), m_currentValue(0)
{
    m_acceptor.open(tcp::v4());
    boost::asio::socket_base::reuse_address option(true);
    m_acceptor.set_option(option);

    m_acceptor.bind(tcp::endpoint(tcp::v4(), 4082));
    m_acceptor.listen(100);

    LOG_NOTIFICATION() << "server port=" << 4082;
}

Server::~Server()
{
    m_isAlive = false;

    // ��O���o�Ȃ��悤�A�O�̂���error���g���Ă��܂��B
    boost::system::error_code error;
    m_acceptor.cancel(error);
    m_acceptor.close(error);

    if (m_serviceThread != nullptr) {
        m_serviceThread->join();
        m_serviceThread.reset();
    }
}

/**
 * @brief �������̃X���b�h���J�n���܂��B
 */
void Server::startThread()
{
    if (m_serviceThread != nullptr) {
        throw std::logic_error("m_serviceThread�͂��łɏ���������Ă��܂��B");
    }

    // IO�֌W�̏������J�n���܂��B
    m_serviceThread.reset(new boost::thread(&Server::serviceThreadMain, this));
    startAccept();

    // �v�l�p�̃X���b�h���J�n���܂��B
    //m_iterateThread.reset(new boost::thread(&Server::iterateThreadMain, this));
}

/**
 * @brief IO�X���b�h�̃��C�����\�b�h�ł��B
 */
void Server::serviceThreadMain()
{
    while (m_isAlive) {
        try {
            boost::system::error_code error;

            // ���������ʂ��O�ł���΁A�����E�F�C�g�����܂��B
            auto count = m_service.poll_one(error);
            if (count == 0) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(20));
            }

            m_service.reset();
        }
        catch (...) {
            LOG_ERROR() << "Server��IO�X���b�h�ŃG���[���������܂����B";
        }
    }
}

/**
 * @brief �N���C�A���g�̎�M�������J�n���܂��B
 */
void Server::startAccept()
{
    if (!m_isAlive) return;

    try
    {
        shared_ptr<tcp::socket> socket(new tcp::socket(m_service));

        m_acceptor.async_accept(*socket,
            bind(&Server::handleAccept, shared_from_this(),
                 socket, boost::asio::placeholders::error));
    }
    catch (std::exception & ex)
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

/**
 * @brief �N���C�A���g�̎�M��ɌĂ΂�܂��B
 */
void Server::handleAccept(shared_ptr<tcp::socket> socket,
                          boost::system::error_code const & error)
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

    startAccept();
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

    return result;
}

/**
 * @brief �N���C�A���g�̐ڑ���҂��܂��B
 */
void Server::waitClients()
{
    while (m_clientList.size() < CLIENT_SIZE) {
        sleep(200);
    }
}

/**
 * @brief �w���ID�����N���C�A���g�ɃR�}���h�𑗐M���܂��B
 */
void Server::sendCommand(int clientId, shared_ptr<CommandPacket> command)
{
    if (command == nullptr) {
        throw std::invalid_argument("command");
    }

    auto & client = m_clientList[clientId];
    client->sendCommand(command, true);
}

/**
 * @brief ���ׂẴN���C�A���g�ɃR�}���h�𑗐M���܂��B
 */
void Server::sendCommandAll(shared_ptr<CommandPacket> command)
{
    if (command == nullptr) {
        throw std::invalid_argument("command");
    }

    FOREACH_CLIENT (client) {
        client->sendCommand(command, true);
    }
}

/**
 * @brief �T�[�o�[���R�}���h��ǉ����܂��B
 */
void Server::addServerCommand(shared_ptr<ServerCommand> scommand)
{
    if (scommand == nullptr) {
        throw std::invalid_argument("scommand");
    }

    LOCK (m_serverCommandGuard) {
        m_serverCommandList.push_back(scommand);
    }
}

/**
 * @brief �T�[�o�[���R�}���h���폜���܂��B
 */
void Server::removeServerCommand(shared_ptr<ServerCommand> scommand)
{
    LOCK (m_serverCommandGuard) {
        m_serverCommandList.remove(scommand);
    }
}

/**
 * @brief ���ɏ�������T�[�o�[���R�}���h���擾���܂��B
 */
shared_ptr<ServerCommand> Server::getNextServerCommand() const
{
    LOCK (m_serverCommandGuard) {
        if (m_serverCommandList.empty()) {
            return shared_ptr<ServerCommand>();
        }

        return m_serverCommandList.front();
    }
}

} // namespace server
} // namespace godwhale
