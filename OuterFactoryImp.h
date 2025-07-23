#ifndef _OUTER_FACTORY_IMP_H_
#define _OUTER_FACTORY_IMP_H_

#include <string>
#include <map>
#include "servant/Application.h"
#include "globe.h"
#include "OuterFactory.h"
#include "LogComm.h"
#include "RsaEncrypt.h"
#include "ClubProto.h"

//wbl
#include <wbl/regex_util.h>

//配置服务
#include "DBAgentServant.h"
#include "OrderServant.h"
#include "HallServant.h"
#include "SocialServant.h"
#include "ConfigServant.h"
#include "Log2DBServant.h"
#include "PushServant.h"

//
using namespace dataproxy;
using namespace dbagent;

//时区
#define ONE_DAY_TIME (24*60*60)
#define ZONE_TIME_OFFSET (8*60*60)

//
class OuterFactoryImp;
typedef TC_AutoPtr<OuterFactoryImp> OuterFactoryImpPtr;

typedef struct TWalletOrderInfo
{
    long uid;
    string inOrderID;//外部订单号
    string outOrderID;//外部订单号
    int tradeType;// 交易类型 1:充值 2：提现
    int channelType;//渠道类型 1：usdt， 2 ：人工
    int orderType;//订单类型 0 充值  1转账
    int status;//订单状态
    int sandbox;//是否测试订单
    string applyAmount;//申请金额
    string amount;//支付(提现)金额
    long clubId;//俱乐部id
    int addrType;//地址类型 1：usdt， 2 ：汇旺
    string productID;//产品id
    string fromAddr;//玩家充值地址
    string toAddr;//平台地址
    string payTime;//支付时间
    string createTime;//订单创建时间
    string reviewer;
    string auditTime;
    string remark;
    TWalletOrderInfo(): uid(0), inOrderID(""), outOrderID(""), tradeType(0), channelType(0), orderType(0), status(0),
                        sandbox(0), applyAmount(""), amount(""), clubId(0), addrType(0), productID(""),
                        fromAddr(""),toAddr(""), payTime(""), createTime(""), reviewer(""), auditTime(""), remark("")
    {
    }
} WalletOrderInfo;

/**
 * 外部工具接口对象工厂
 */
class OuterFactoryImp : public OuterFactory
{
private:
    /**
     *
    */
    OuterFactoryImp();

    /**
     *
    */
    ~OuterFactoryImp();

    /**
     *
     */
    friend class OrderServantImp;

    /**
     *
     */
    friend class OrderServer;

public:
    //框架中用到的outer接口(不能修改):
    const OuterProxyFactoryPtr &getProxyFactory() const
    {
        return _pProxyFactory;
    }

    tars::TC_Config &getConfig() const
    {
        return *_pFileConf;
    }

public:
    //读取所有配置
    void load();
    //代理配置
    void readPrxConfig();
    //打印代理配置
    void printPrxConfig();

private:
    //
    void createAllObject();
    //
    void deleteAllObject();

public:
    //游戏配置服务代理
    const config::ConfigServantPrx getConfigServantPrx();
    //金猪服务代理
    const Social::SocialServantPrx getSocialServantPrx(const long uid);
    //数据库代理服务代理
    const DBAgentServantPrx getDBAgentServantPrx(const long uid);
    //数据库代理服务代理
    const DBAgentServantPrx getDBAgentServantPrx(const string key);
    //广场服务代理
    const hall::HallServantPrx getHallServantPrx(const long uid);
    //广场服务代理
    const hall::HallServantPrx getHallServantPrx(const string key);
    //日志入库服务代理
    const DaqiGame::Log2DBServantPrx getLog2DBServantPrx(const long uid);
    //
    const push::PushServantPrx getPushServantPrx(const long uid);
    //
    const config::DBConf &getDBConfig()
    {
        return dbConf;
    }
    void readDBConfig();
    //
    string getIp(const string &domain); 
   
    int isSandBox()
    {
        return _sandBox;
    }

    string getWalletCallbackUrl()
    {
        return _walletCallbackUrl;
    }

    string getWalletAddress()
    {
        return _walletAddress;
    }

    string getMd5Key()
    {
        return _md5Key;
    }

    string getMerchantId()
    {
        return _merchantId;
    }

    long getWalletMinAmount()
    {
        return _minAmount;
    }

    long getWalletMaxAmount()
    {
        return _maxAmount;
    }

    long getWalletMaxDayAmount()
    {
        return _maxDayAmount;
    }

    int getWalletHandingFee()
    {
        return _handingFee;
    }

    int getLimitRecord()
    {
        return _limitRecord;
    }

    int getGameId()
    {
        return _gameId;
    }

    string getRobotId()
    {
        return _robotId;
    }

    double getExchangeRate()
    {
        return _exchangeRate / 100.00;
    }

    map<string, string>& getCallMe()
    {
        return _callMe;
    }

    vector<map<string, string>> &getMalllConfig()
    {
        return mallConfigRaw;
    }

    int getGoodsTransferRate()
    {
       return _goodsTransferRate;
    }

    int updateClubCoin(const long lPlayerID, const int club_id, const string& product_id, const double discount);

    int updateClubCoinByManual(const long lPlayerID, const int club_id, const long count);

    void getFlowRecordTypeList(int iType, std::vector<int> &vResult);

    int getFlowRecordTypeByID(int recordID);

    void asyncLog2DB(const int64_t uid, const DaqiGame::TLog2DBReq &req);

    void asyncPushBindTg(const int64_t uid, const push::PushMsgReq &msg);

    int asyncWalletMailNotify(const long lUid, const int mailType, std::vector<string> vParam);

public:
    //格式化时间
    string GetTimeFormat();
    //
    time_t convertTimeStr2TimeStamp(string timeStr);
    //
    std::string GetTimeDayLater(int days);
    //
    std::string GetTimeMonthLater(int days);
    //
    std::string GetTimeDayZero();
    //获得时间秒数
    int GetTimeTick(const string &str);
    //
    int GetTimeDays();
    //
    int GetMonthDays(const string& str);
public:
    //拆分字符串成整形
    int splitInt(string szSrc, vector<int> &vecInt);
    //拆分字符串
    vector<std::string> split(const string &str, const string &pattern);

private:
    //读写锁，防止脏读
    wbl::ReadWriteLocker m_rwlock;

private:
    //框架用到的共享对象(不能修改):
    tars::TC_Config *_pFileConf;
    //
    OuterProxyFactoryPtr _pProxyFactory;

private:
    //数据库代理服务
    std::string _DBAgentServantObj;
    DBAgentServantPrx _DBAgentServerPrx;

    std::string _HallServantObj;
    hall::HallServantPrx _HallServerPrx;

    std::string _SocialServantObj;
    Social::SocialServantPrx _SocialServerPrx;

    //
    std::string _ConfigServantObj;
    config::ConfigServantPrx _ConfigServantPrx;

    //日志入库服务
    std::string _Log2DBServantObj;
    DaqiGame::Log2DBServantPrx _Log2DBServerPrx;

    //推送服务
    std::string _PushServantObj;
    push::PushServantPrx _PushServerPrx;

private:
    config::DBConf dbConf; //数据源配置

private:

    //wallet
    int _debug;
    string _walletRsaPublicKey;//钱包rsa 公钥
    string _walletRsaPrivateKey;
    string _walletUrl;//钱包接口地址
    string _walletCallbackUrl;
    string _md5Key;
    string _merchantId;
    long _minAmount; //最低充值额度
    long _maxAmount; //最高充值额度
    int _handingFee; //手续费
    int _limitRecord;//查询页显示记录数
    long _maxDayAmount;//每日提现金额限制
    int _gameId; //游戏id
    string _robotId;//收银台机器人id
    int _exchangeRate;//币种转换比例

    bool _sandBox;
    string _bakUrl;//备用测试url
    string _walletAddress;//官方钱包地址

    map<int, string> _mailContent;//邮件模板
    map<int, string> _mailTitle;//邮件模板

    int _goodsTransferRate;//资金转移手续费
    string _flowRecordStartDate;
    map<int, std::vector<int>> _flowRecordCategory;// 流水统计类型

    std::map<string, string> _callMe;//联系方式

    vector<map<string, string>> mallConfigRaw; //原始配置,未加工
};

////////////////////////////////////////////////////////////////////////////////
#endif


