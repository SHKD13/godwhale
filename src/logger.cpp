
#include "precomp.h"
#include "stdinc.h"
#include "logger.h"

#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>   
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/attributes/current_process_name.hpp>

namespace godwhale {

using namespace boost::log;
using namespace boost::posix_time;

/**
 * @brief �O���[�o�����K�[��ݒ肷��B
 */
BOOST_LOG_GLOBAL_LOGGER_INIT(Logger, sources::severity_logger_mt<SeverityLevel>)
{
    //���O���R�[�h�̑�����ݒ肷��
    auto r = sources::severity_logger_mt<SeverityLevel>();
    
    //[���O�Ɋ܂܂�鑮����ݒ肷��]
    //�ȉ��̋��ʑ�����`�֐��Œ�`���鎖���\�ł���B
    //boost::log::add_common_attributes();
    //
    //�����ł́A�Ǝ��ɕK�v�ȑ����̒�`���s��
    r.add_attribute("TimeStamp", attributes::local_clock());
    r.add_attribute("ProcessID", attributes::current_process_id());
    r.add_attribute("Process", attributes::current_process_name());
    r.add_attribute("ThreadID", attributes::current_thread_id());

    return std::move(r);
}

/**
 * @brief �t�@�C���p�X���疼�O�݂̂����o���܂��B
 */
std::string getLogFileName(const std::string &filepath)
{
    char separator =
#if defined(_WIN32)
        '\\';
#else
        '/';
#endif

    auto i = filepath.find_last_of(separator);
    return (i != std::string::npos ? filepath.substr(i+1) : filepath);
}

/**
 * @brief ���O�o�͗p�̊֐��������o���܂��B
 */
std::string getLogFuncName(const std::string &funcname)
{
    auto i = funcname.find_last_of(':');
    return (i != std::string::npos ? funcname.substr(i+1) : funcname);
}

/**
 * @brief ���O�̏o�̓��x�������擾���܂��B
 */
std::string getSeverityName(SeverityLevel severity)
{
    switch (severity) {
    case Debug:
        return "DEBUG";
    case Notification:
        return "NOTIFICATION";
    case Warning:
        return "WARNING";
    case Error:
        return "ERROR";
    case Critical:
        return "CRITICAL";
    }

    return "UNKNOWN";
}

/**
 * @brief ���O�t�@�C�����ɂ̓^�C���X�^���v���g���܂��B
 */
static std::string getLogName()
{
    time_facet *f = new time_facet("%Y%m%d_%H%M%S.log");

    std::ostringstream oss;
    oss.imbue(std::locale(oss.getloc(), f));

    oss << second_clock::local_time();
    return oss.str();
}

/**
 * @brief �������Ȃ�deleter�ł��B
 */
struct empty_deleter
{
    typedef void result_type;
    void operator() (const volatile void*) const {}
};

/**
 * @brief �΋ǊJ�n���Ƀ��O�t�@�C���̏��������s���܂��B
 */
void initializeLog()
{
    typedef sinks::text_ostream_backend text_backend;
    auto backend = make_shared<text_backend>();
    backend->auto_flush(true);

    // �R���\�[���o��
#if !defined(USI)
    auto os = shared_ptr<std::ostream>(&std::cout, empty_deleter());
    backend->add_stream(os);
#endif

    // �t�@�C���o��
    auto logpath =
#if defined(GODWHALE_SERVER)
        "server.log";
#else
        "client.log";
#endif
    auto fs = shared_ptr<std::ostream>(new std::ofstream(logpath, std::ios::app));
    backend->add_stream(fs);

    // �ʏ탍�O
    auto normal = make_shared< sinks::synchronous_sink<text_backend> >(backend);
    //error->set_filter(Severity >= Warning);
    normal->set_formatter
        ( expressions::format("[%1%]: %2%")
        % expressions::format_date_time<ptime>
            ("TimeStamp", "%Y-%m-%d %H:%M:%S")
        % expressions::message);
    
    // �G���[�p�o��
    auto error = make_shared< sinks::synchronous_sink<text_backend> >(backend);
    error->set_filter(Severity >= Warning);
    error->set_formatter
        ( expressions::format("[%1%]:%2%(%3%):%4%:%5%: %6%")
        % expressions::format_date_time<ptime>
            ("TimeStamp", "%Y-%m-%d %H:%M:%S")
        % expressions::attr<std::string>("SrcFile")
        % expressions::attr<std::string>("RecordLine")
        % expressions::attr<std::string>("CurrentFunction")
        % expressions::attr<std::string>("SeverityName")
        % expressions::message);

    core::get()->remove_all_sinks();
    core::get()->add_sink(normal);
    core::get()->add_sink(error);
}

} // namespace godwhale
