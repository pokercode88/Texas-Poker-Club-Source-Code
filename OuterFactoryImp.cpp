#include <sstream>
#include "OuterFactoryImp.h"
#include "LogComm.h"
#include "OrderServer.h"
#include "util/tc_hash_fun.h"

using namespace wbl;

namespace
{
    template<typename T>
    void mapInsert(T &m, const typename T::key_type &key, const typename T::mapped_type &value)
    {
        m.insert(make_pair(key, value));
    }

    template<typename T>
    std::string mToString(const T &param)
    {
        ostringstream os;
        os << param;
        return os.str();
    }

    template<>
    std::string mToString<std::string>(const std::string &param)
    {
        ostringstream os;
        os << "\"" << param << "\"";
        return os.str();
    }

    template<typename Key, typename Value>
    std::string mToString(const map<Key, Value> &param)
    {
        ostringstream os;
        os << "{";
        size_t len = param.size();
        size_t idx = 0;
        for (auto &elem : param)
        {
            os << "\"" << elem.first << "\":" << mToString(elem.second);
            if (idx != len - 1)
            {
                os << ",";
            }
            idx += 1;
        }
        os << "}";
        return os.str();
    }
}

/**
 *
*/
OuterFactoryImp::OuterFactoryImp() : _pFileConf(NULL)
{
    createAllObject();
}

/**
 *
*/
OuterFactoryImp::~OuterFactoryImp()
{
    deleteAllObject();
}

void OuterFactoryImp::deleteAllObject()
{
    if (_pFileConf)
    {
        delete _pFileConf;
        _pFileConf = NULL;
    }
}

void OuterFactoryImp::createAllObject()
{
    try
    {
        //
        deleteAllObject();

        //本地配置文件
        _pFileConf = new tars::TC_Config();
        if (!_pFileConf)
        {
            ROLLLOG_ERROR << "create config parser fail, ptr null." << endl;
            terminate();
        }

        //tars代理Factory,访问其他tars接口时使用
        _pProxyFactory = new OuterProxyFactory();
        if ((long int)NULL == _pProxyFactory)
        {
            ROLLLOG_ERROR << "create outer proxy factory fail, ptr null." << endl;
            terminate();
        }

        FDLOG_RECHARGE_LOG_FORMAT;
        LOG_DEBUG << "init proxy factory succ." << endl;

        //读取所有配置
        load();
    }
    catch (TC_Exception &ex)
    {
        LOG->error() << ex.what() << endl;
    }
    catch (exception &e)
    {
        LOG->error() << e.what() << endl;
    }
    catch (...)
    {
        LOG->error() << "unknown exception." << endl;
    }

    return;
}

//读取所有配置
void OuterFactoryImp::load()
{
    __TRY__

    //拉取远程配置
    g_app.addConfig(ServerConfig::ServerName + ".conf");

    wbl::WriteLocker lock(m_rwlock);

    _pFileConf->parseFile(ServerConfig::BasePath + ServerConfig::ServerName + ".conf");
    LOG_DEBUG << "init config file succ:" << ServerConfig::BasePath + ServerConfig::ServerName + ".conf" << endl;

    //代理配置
    readPrxConfig();
    printPrxConfig();

    readDBConfig();

    //mall
    mallConfigRaw.clear();
    getConfigServantPrx()->ListMallConfigRaw(mallConfigRaw);

    CRSAEnCryptSingleton::getInstance()->init(_walletUrl, _bakUrl, _walletRsaPublicKey, _walletRsaPrivateKey, _debug);
    __CATCH__
}

//代理配置
void OuterFactoryImp::readPrxConfig()
{
   
    //配置服务
    _DBAgentServantObj = (*_pFileConf).get("/Main/Interface/DBAgentServer<ProxyObj>", "");
    _HallServantObj = (*_pFileConf).get("/Main/Interface/HallServer<ProxyObj>", "");
    _SocialServantObj = (*_pFileConf).get("/Main/Interface/SocialServer<ProxyObj>", "");
    _Log2DBServantObj = (*_pFileConf).get("/Main/Interface/Log2DBServer<ProxyObj>", "");
    _ConfigServantObj = (*_pFileConf).get("/Main/Interface/ConfigServer<ProxyObj>", "");
    _PushServantObj = (*_pFileConf).get("/Main/Interface/PushServer<ProxyObj>", "");


    //钱包相关
    _debug = S2L((*_pFileConf).get("/Main/wallet<debug>", "0")); 
    _walletRsaPublicKey = (*_pFileConf).get("/Main/wallet<publicKey>", "");
    _walletRsaPrivateKey = (*_pFileConf).get("/Main/wallet<privateKey>", "");
    _walletUrl = (*_pFileConf).get("/Main/wallet<url>", "");
    _bakUrl = (*_pFileConf).get("/Main/wallet<bak_url>", "");
    _walletCallbackUrl = (*_pFileConf).get("/Main/wallet<callbackurl>", "");
    _md5Key = (*_pFileConf).get("/Main/wallet<md5Key>", "");
    _merchantId = (*_pFileConf).get("/Main/wallet<merchantId>", "");
    _minAmount = S2L((*_pFileConf).get("/Main/wallet<minAmount>", "0")); //最低充值额度
    _maxAmount = S2L((*_pFileConf).get("/Main/wallet<maxAmount>", "0")); //最高充值额度
    _handingFee = S2I((*_pFileConf).get("/Main/wallet<handingFee>", "0")); //手续费
    _limitRecord = S2I((*_pFileConf).get("/Main/wallet<limitRecord>", "10")); //查询页显示记录数
    _maxDayAmount = S2I((*_pFileConf).get("/Main/wallet<maxDayAmount>", "10")); //每日提现金额限制

    _gameId = S2I((*_pFileConf).get("/Main/wallet<gameId>", "10001")); //游戏id
    _robotId = (*_pFileConf).get("/Main/wallet<robotid>", ""); //游戏id
    _exchangeRate = S2I((*_pFileConf).get("/Main/wallet<exchangeRate>", "10000"));

    _walletAddress = (*_pFileConf).get("/Main/wallet<walletAddress>", "");
    _sandBox = S2I((*_pFileConf).get("/Main/wallet<sandbox>", "0")) == 1;

    //邮件模板
    _mailContent.clear();
    _mailTitle.clear();
    auto vecDomainKey = (*_pFileConf).getDomainVector("/Main/mail");
    for (auto &domain : vecDomainKey)
    {
        string subDomain = "/Main/mail/" + domain;
        string content = (*_pFileConf).get(subDomain + "<content>");
        string title = (*_pFileConf).get(subDomain + "<title>");
        _mailContent.insert(std::make_pair(TC_Common::strto<int>(domain), content));
        _mailTitle.insert(std::make_pair(TC_Common::strto<int>(domain), title));
    }

    //流水统计类型
    _flowRecordCategory.clear();
    auto vecDomainKey1 = (*_pFileConf).getDomainVector("/Main/flowrecord");
    for (auto &domain : vecDomainKey1)
    {
        string subDomain = "/Main/flowrecord/" + domain;
        string content = (*_pFileConf).get(subDomain + "<content>");
        ROLLLOG_DEBUG << "domain1=" << domain << ", content1: "<< content << endl;
        auto resule = split(content, "|");
        std::vector<int> vT;
        for(auto i : resule)
        {
            vT.push_back(S2I(i));
        }
        
        _flowRecordCategory.insert(std::make_pair(TC_Common::strto<int>(domain), vT));

    }
    _flowRecordStartDate = (*_pFileConf).get("/Main/flowrecord<start_date>", "2022-07-05");
    _goodsTransferRate = S2I((*_pFileConf).get("/Main/flowrecord<good_transfer_rate>", "5"));

    //客服
    _callMe.clear();
    auto vecDomainKey2 = (*_pFileConf).getDomainVector("/Main/callme");
    for (auto &domain : vecDomainKey2)
    {
        string subDomain = "/Main/callme/" + domain;
        string name = (*_pFileConf).get(subDomain + "<name>");
        string phone = (*_pFileConf).get(subDomain + "<phone>"); 
        _callMe.insert(std::make_pair(name, phone));
    }
}

//打印代理配置
void OuterFactoryImp::printPrxConfig()
{
    FDLOG_CONFIG_INFO << "_DBAgentServantObj ProxyObj:" << _DBAgentServantObj << endl;
    FDLOG_CONFIG_INFO << "_HallServantObj ProxyObj:" << _HallServantObj << endl;
    FDLOG_CONFIG_INFO << "_SocialServantObj ProxyObj:" << _SocialServantObj << endl;
    FDLOG_CONFIG_INFO << "_ConfigServantObj ProxyObj:" << _ConfigServantObj << endl;
    FDLOG_CONFIG_INFO << "_Log2DBServantObj ProxyObj : " << _Log2DBServantObj << endl;
    FDLOG_CONFIG_INFO << "_walletRsaPublicKey: " << _walletRsaPublicKey << endl;
    FDLOG_CONFIG_INFO << "_walletRsaPrivateKey: " << _walletRsaPrivateKey << endl;
    FDLOG_CONFIG_INFO << "_walletUrl: " << _walletUrl << endl;
}

const config::ConfigServantPrx OuterFactoryImp::getConfigServantPrx()
{
    if (!_ConfigServantPrx)
    {
        _ConfigServantPrx = Application::getCommunicator()->stringToProxy<config::ConfigServantPrx>(_ConfigServantObj);
        LOG_DEBUG << "Init _ConfigServantObj succ, _ConfigServantObj: " << _ConfigServantObj << endl;
    }

    return _ConfigServantPrx;
}

// 金猪代理
const Social::SocialServantPrx OuterFactoryImp::getSocialServantPrx(const long uid)
{
    if (!_SocialServerPrx)
    {
        _SocialServerPrx = Application::getCommunicator()->stringToProxy<Social::SocialServantPrx>(_SocialServantObj);
        ROLLLOG_DEBUG << "Init _SocialServantObj succ, _SocialServantObj:" << _SocialServantObj << endl;
    }

    if (_SocialServerPrx)
    {
        return _SocialServerPrx->tars_hash(uid);
    }

    return NULL;
}

//数据库代理服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const long uid)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj:" << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(uid);
    }

    return NULL;
}

//数据库代理服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const string key)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj:" << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(tars::hash<string>()(key));
    }

    return NULL;
}

//
const hall::HallServantPrx OuterFactoryImp::getHallServantPrx(const long uid)
{
    if (!_HallServerPrx)
    {
        _HallServerPrx = Application::getCommunicator()->stringToProxy<hall::HallServantPrx>(_HallServantObj);
        ROLLLOG_DEBUG << "Init _HallServantObj succ, _HallServantObj:" << _HallServantObj << endl;
    }

    if (_HallServerPrx)
    {
        return _HallServerPrx->tars_hash(uid);
    }

    return NULL;
}

//
const hall::HallServantPrx OuterFactoryImp::getHallServantPrx(const string key)
{
    if (!_HallServerPrx)
    {
        _HallServerPrx = Application::getCommunicator()->stringToProxy<hall::HallServantPrx>(_HallServantObj);
        ROLLLOG_DEBUG << "Init _HallServantObj succ, _HallServantObj:" << _HallServantObj << endl;
    }

    if (_HallServerPrx)
    {
        return _HallServerPrx->tars_hash(tars::hash<string>()(key));
    }

    return NULL;
}

//日志入库服务代理
const DaqiGame::Log2DBServantPrx OuterFactoryImp::getLog2DBServantPrx(const long uid)
{
    if (!_Log2DBServerPrx)
    {
        _Log2DBServerPrx = Application::getCommunicator()->stringToProxy<DaqiGame::Log2DBServantPrx>(_Log2DBServantObj);
        ROLLLOG_DEBUG << "Init _Log2DBServantObj succ, _Log2DBServantObj : " << _Log2DBServantObj << endl;
    }

    if (_Log2DBServerPrx)
    {
        return _Log2DBServerPrx->tars_hash(uid);
    }

    return NULL;
}

//
const push::PushServantPrx OuterFactoryImp::getPushServantPrx(const long uid)
{
    if (!_PushServerPrx)
    {
        _PushServerPrx = Application::getCommunicator()->stringToProxy<push::PushServantPrx>(_PushServantObj);
        ROLLLOG_DEBUG << "Init _PushServantObj succ, _PushServantObj : " << _PushServantObj << endl;
    }

    if (_PushServerPrx)
    {
        return _PushServerPrx->tars_hash(uid);
    }

    return NULL;
}

void OuterFactoryImp::asyncPushBindTg(const int64_t uid, const push::PushMsgReq &msg)
{
    getPushServantPrx(uid)->async_pushMsg(NULL, msg);
}

//日志入库
void OuterFactoryImp::asyncLog2DB(const int64_t uid, const DaqiGame::TLog2DBReq &req)
{
    getLog2DBServantPrx(uid)->async_log2db(NULL, req);
}

//格式化时间
string OuterFactoryImp::GetTimeFormat()
{
    string sFormat("%Y-%m-%d %H:%M:%S");
    time_t t = time(NULL);
    auto pTm = localtime(&t);
    if (!pTm)
        return "";

    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return string(sTimeString);
}

//几天之前的时间
std::string OuterFactoryImp::GetTimeDayLater(int days)
{
    std::string sFormat("%Y-%m-%d %H:%M:%S");
    time_t t = time(NULL) - 24 * 3600 * days;
    struct tm *pTm = localtime(&t);
    if (pTm == NULL)
    {
        return "";
    }

    ///
    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return std::string(sTimeString);
}

//几天之前的月份
std::string OuterFactoryImp::GetTimeMonthLater(int days)
{
    string date = GetTimeDayLater(days);
    if(date.size() < 19)
    {
        return "";
    }
    return date.substr(0, 4) + date.substr(5, 2);
}

//今天凌晨
std::string OuterFactoryImp::GetTimeDayZero()
{
    std::string sFormat("%Y-%m-%d %H:%M:%S");
    time_t t = time(NULL) - (time(NULL) % (24 * 3600));
    struct tm *pTm = localtime(&t);
    if (pTm == NULL)
    {
        return "";
    }

    ///
    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return std::string(sTimeString);
}

time_t OuterFactoryImp::convertTimeStr2TimeStamp(string timeStr)
{
    struct tm timeinfo;
    strptime(timeStr.c_str(), "%Y-%m-%d %H:%M:%S",  &timeinfo);
    time_t timeStamp = mktime(&timeinfo);
    printf("timeStamp=%ld\n",timeStamp);
    return timeStamp;
}

//获得时间秒数
int OuterFactoryImp::GetTimeTick(const string &str)
{
    if (str.empty())
        return 0;

    struct tm tm_time;
    string sFormat("%Y-%m-%d %H:%M:%S");
    strptime(str.c_str(), sFormat.c_str(), &tm_time);
    return mktime(&tm_time);
}

//获取时间天数
int OuterFactoryImp::GetTimeDays()
{
    long cur_zero_time = TNOW - TNOW % (24 * 3600);
    return (cur_zero_time - GetTimeTick(_flowRecordStartDate + " 00:00:00")) / (24 * 3600) + 1;
}

//获取本月第几天
int OuterFactoryImp::GetMonthDays(const string& str)
{
    if(str.size() < 19)
    {
        return 0;
    }
    int start_index = str[8] == '0' ? 9 : 8;
    return S2I(str.substr(start_index, 2));
}

//拆分字符串成整形
int OuterFactoryImp::splitInt(string szSrc, vector<int> &vecInt)
{
    split_int(szSrc, "[ \t]*\\|[ \t]*", vecInt);
    return 0;
}

//拆分字符串
vector<std::string> OuterFactoryImp::split(const string &str, const string &pattern)
{
    return SEPSTR(str, pattern);
}

////////////////////////////////////////////////////////////////////////////////

int OuterFactoryImp::updateClubCoin(const long lPlayerID, const int club_id, const string& product_id, const double discount)
{
    long count = 0;
    for(auto cfg : mallConfigRaw)
    {
        if(cfg["product_id"] != product_id)
        {
            continue;
        }

        //兑换组
        auto exchangeGroup = split(cfg["exchangeGroup"], "|");
        for(auto item : exchangeGroup)
        {
            auto vecSub = split(item, ":");
            if(vecSub.size() != 2 || S2L(vecSub[0]) != 30000)
            {
                continue;
            }
            count = S2L(vecSub[1]) * discount;
        }
    }
    Club::InnerClubInfoCoinReq req;
    req.uId = lPlayerID;
    req.cId = club_id;
    req.flag = 1;
    req.amount = count;

    LOG_DEBUG<< "updateClubCoin req:" << printTars(req) << endl;
    getSocialServantPrx(lPlayerID)->async_InnerClubInfoCoin(NULL, req);

    return 0;
}

int OuterFactoryImp::updateClubCoinByManual(const long lPlayerID, const int club_id, const long count)
{
    Club::InnerClubInfoCoinReq req;
    req.uId = lPlayerID;
    req.cId = club_id;
    req.flag = 1;
    req.amount = count;

    LOG_DEBUG<< "updateClubCoin req:" << printTars(req) << endl;
    getSocialServantPrx(lPlayerID)->async_InnerClubInfoCoin(NULL, req);

    return 0;
}

//邮件通知
int OuterFactoryImp::asyncWalletMailNotify(const long lUid, const int mailType, std::vector<string> vParam)
{
    mail::TSendMailReq tSendMailReq;
    tSendMailReq.uid = lUid;
    mail::TMailData &mailData = tSendMailReq.data;
    mailData.type = mail::E_MAIL_TYPE_USER;//个人邮件

    auto ittitle = _mailTitle.find(mailType);
    if(ittitle != _mailTitle.end())
    {
       mailData.title = ittitle->second;
    }

    auto it = _mailContent.find(mailType);
    if(it == _mailContent.end())
    {
        ROLLLOG_ERROR<<"mail template not exist. mailType: "<< mailType << endl;
        return 0;
    }

    mailData.content = it->second;
    for(unsigned int i = 0; i < vParam.size(); i++)
    {
        size_t pos = mailData.content.find("&");
        if( pos != std::string::npos)
        {
            mailData.content.replace(pos, 1, vParam[i]);
        }
    }

    mail::TSendMailResp tSendMailResp;
    int iRet = getHallServantPrx(lUid)->sendMailToUserFromSystem(tSendMailReq, tSendMailResp);
    if(iRet != 0)
    {
        LOG_ERROR<< "send mail err. lUid: "<< lUid<< endl;
        return iRet;
    }
    return 0;
}

void OuterFactoryImp::getFlowRecordTypeList(int iType, std::vector<int> &vResult)
{
    auto it = _flowRecordCategory.find(iType);
    if(it != _flowRecordCategory.end())
    {
        vResult.insert(vResult.begin(), it->second.begin(), it->second.end());
    }
    else
    {
        for(auto item : _flowRecordCategory)
        {
            vResult.insert(vResult.begin(), item.second.begin(), item.second.end());
        }
    }
}

int OuterFactoryImp::getFlowRecordTypeByID(int recordID)
{
    for(auto item : _flowRecordCategory)
    {
        auto it = std::find(item.second.begin(), item.second.end(), recordID);
        if(it != item.second.end())
        {
            return item.first;
        }
    }
    return -1;
}


//域名解析
string OuterFactoryImp::getIp(const string &domain)
{
    if(domain.length() == 0)
    {
        return "";
    }

    struct hostent host = *gethostbyname(domain.c_str());
    for (int i = 0; host.h_addr_list[i]; i++)
    {
        string ip = inet_ntoa(*(struct in_addr *)host.h_addr_list[i]);
        return ip;
    }

    return "";
}


// 读取db配置
void OuterFactoryImp::readDBConfig()
{
    dbConf.Domain = (*_pFileConf).get("/Main/db<domain>", "");
    dbConf.Host = (*_pFileConf).get("/Main/db<host>", "");
    dbConf.port = (*_pFileConf).get("/Main/db<port>", "3306");
    dbConf.user = (*_pFileConf).get("/Main/db<user>", "tars");
    dbConf.password = (*_pFileConf).get("/Main/db<password>", "tars2015");
    dbConf.charset = (*_pFileConf).get("/Main/db<charset>", "utf8");
    dbConf.dbname = (*_pFileConf).get("/Main/db<dbname>", "");

    //域名
    if (dbConf.Domain.length() > 0)
    {
        string szHost = getIp(dbConf.Domain);
        if (szHost.length() > 0)
        {
            dbConf.Host = szHost;
            ROLLLOG_DEBUG << "get host by domain, Domain: " << dbConf.Domain << ", szHost: " << szHost << endl;
        }
    }
}
