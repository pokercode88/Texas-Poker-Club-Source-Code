#include "Processor.h"
#include "globe.h"
#include "LogComm.h"
#include "ServiceDefine.h"
#include "util/tc_hash_fun.h"
#include "uuid.h"
#include "OrderServer.h"

#include <iomanip>
#include <sstream>


/**
 * 
*/
Processor::Processor()
{
}

/**
 * 
*/
Processor::~Processor()
{
}

//ORDER_IOS      = 87,  //tb_ios_order
//查询
int Processor::selectIOSOrder()
{;

    return 0;
}

//增加
int Processor::UpdateIOSOrder()
{
    return 0;
}

//直接查询数据库
int Processor::readDataFromDB(long uid, const string& table_name, const std::vector<string>& col_name, const std::vector<std::vector<string>>& whlist, const string& order_col, int page, int limit_num,  dbagent::TDBReadRsp &dataRsp)
{
    int iRet = 0;
    dbagent::TDBReadReq rDataReq;
    rDataReq.keyIndex = 0;
    rDataReq.queryType = dbagent::E_SELECT;
    rDataReq.tableName = table_name;

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : col_name)
    {
        tfield.colName = item;
        fields.push_back(tfield);
    }
    rDataReq.fields = fields;

    //where条件组
    if(!whlist.empty())
    {
        vector<dbagent::ConditionGroup> conditionGroups;
        dbagent::ConditionGroup conditionGroup;
        conditionGroup.relation = dbagent::AND;
        vector<dbagent::Condition> conditions;
        for(auto item : whlist)
        {
            if(item.size() != 3)
            {
                continue;
            }
            dbagent::Condition condition;
            condition.condtion = item[1] == "E_GE" ? dbagent::E_GE : dbagent::E_EQ ;
            condition.colType = dbagent::STRING;
            condition.colName = item[0];
            condition.colValues = item[2];
            conditions.push_back(condition);
        }
        conditionGroup.condition = conditions;
        conditionGroups.push_back(conditionGroup);
        rDataReq.conditions = conditionGroups;
    }
   
    //order by字段
    if(!order_col.empty())
    {
        vector<dbagent::OrderBy> orderBys;
        dbagent::OrderBy orderBy;
        orderBy.sort = dbagent::DESC;
        orderBy.colName = order_col;
        orderBys.push_back(orderBy);
        rDataReq.orderbyCol = orderBys;
    }

    if(limit_num > 0)
    {
        //指定返回的行数的最大值
        rDataReq.limit = limit_num;
        //指定返回的第一行的偏移量
        rDataReq.limit_from = (page -1) * limit_num;
    }

    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->read(rDataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "read data from dbagent failed, rDataReq:" << printTars(rDataReq) << ",dataRsp: " << printTars(dataRsp) << endl;
        return -1;
    }
    return 0;
}

//创建钱包订单
int Processor::createWalletOrder(const string& outOrderID, WalletOrderInfo &orderInfo)
{
    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(WALLET_ORDER) + ":" + outOrderID;
    wdataReq.operateType = E_REDIS_INSERT;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    wdataReq.clusterInfo.frageFactor = tars::hash<string>()(outOrderID);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "uid";
    tfield.colType = BIGINT;
    tfield.colValue = L2S(orderInfo.uid);
    fields.push_back(tfield);
    tfield.colName = "inOrderID";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.inOrderID;
    fields.push_back(tfield);
    tfield.colName = "outOrderID";
    tfield.colType = STRING;
    tfield.colValue = outOrderID;
    fields.push_back(tfield);
    tfield.colName = "tradeType";
    tfield.colType = INT;
    tfield.colValue = I2S(orderInfo.tradeType);
    fields.push_back(tfield);
    tfield.colName = "channelType";
    tfield.colType = INT;
    tfield.colValue = I2S(orderInfo.channelType);
    fields.push_back(tfield);
    tfield.colName = "orderType";
    tfield.colType = INT;
    tfield.colValue = I2S(orderInfo.orderType);
    fields.push_back(tfield);
    tfield.colName = "sandbox";
    tfield.colType = INT;
    tfield.colValue = I2S(orderInfo.sandbox);
    fields.push_back(tfield);
    tfield.colName = "status";
    tfield.colType = INT;
    tfield.colValue = I2S(orderInfo.status);
    fields.push_back(tfield);
    tfield.colName = "applyAmount";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.applyAmount;
    fields.push_back(tfield);
    tfield.colName = "amount";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.amount;
    fields.push_back(tfield);
    tfield.colName = "addrType";
    tfield.colType = STRING;
    tfield.colValue = I2S(orderInfo.addrType);
    fields.push_back(tfield);
    tfield.colName = "productID";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.productID;
    fields.push_back(tfield);
    tfield.colName = "clubID";
    tfield.colType = BIGINT;
    tfield.colValue = L2S(orderInfo.clubId);
    fields.push_back(tfield);
    tfield.colName = "fromAddr";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.fromAddr;
    fields.push_back(tfield);
    tfield.colName = "toAddr";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.toAddr;
    fields.push_back(tfield);
    tfield.colName = "payTime";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.payTime;
    fields.push_back(tfield);
    tfield.colName = "createTime";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.createTime;
    fields.push_back(tfield);
    tfield.colName = "reviewer";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.reviewer;
    fields.push_back(tfield);
    tfield.colName = "auditTime";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.auditTime.empty() ? g_app.getOuterFactoryPtr()->GetTimeFormat() : orderInfo.auditTime;
    fields.push_back(tfield);
    tfield.colName = "remark";
    tfield.colType = STRING;
    tfield.colValue = orderInfo.remark;
    fields.push_back(tfield);
    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "create wallet order err. iRet:" << iRet << ",wdataRsp:" << printTars(wdataRsp) << endl;
        return -1;
    }
    return 0;
}

int Processor::selectWalletOrder(const string& outOrderID, WalletOrderInfo &orderInfo)
{
    dataproxy::TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(WALLET_ORDER) + ":" + outOrderID;
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(outOrderID);
    dataReq.paraExt.resetDefautlt();
    dataReq.paraExt.queryType = E_SELECT;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "uid";
    tfield.colType = BIGINT;
    fields.push_back(tfield);
    tfield.colName = "inOrderID";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "outOrderID";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "tradeType";
    tfield.colType = INT;
    fields.push_back(tfield);
    tfield.colName = "channelType";
    tfield.colType = INT;
    fields.push_back(tfield);
    tfield.colName = "orderType";
    tfield.colType = INT;
    fields.push_back(tfield);
    tfield.colName = "sandbox";
    tfield.colType = INT;
    fields.push_back(tfield);
    tfield.colName = "status";
    tfield.colType = INT;
    fields.push_back(tfield);
    tfield.colName = "applyAmount";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "amount";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "addrType";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "productID";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "clubID";
    tfield.colType = BIGINT;
    fields.push_back(tfield);
    tfield.colName = "fromAddr";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "toAddr";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "payTime";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "createTime";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "reviewer";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "auditTime";
    tfield.colType = STRING;
    fields.push_back(tfield);
    tfield.colName = "remark";
    tfield.colType = STRING;
    fields.push_back(tfield);

    dataReq.fields = fields;

    TReadDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->redisRead(dataReq, dataRsp);
    //ROLLLOG_DEBUG << "select wallet order, iRet: " << iRet << ", dataRsp: " << printTars(dataRsp) << endl;
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "select wallet order err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -1;
    }
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itaddr = it->begin(); itaddr != it->end(); ++itaddr)
        {
            if (itaddr->colName == "uid")
            {
                orderInfo.uid = S2L(itaddr->colValue);
            }
            if (itaddr->colName == "outOrderID")
            {
                orderInfo.outOrderID = itaddr->colValue;
            }
            if (itaddr->colName == "status")
            {
                orderInfo.status = S2I(itaddr->colValue);
            }
            if (itaddr->colName == "productID")
            {
                orderInfo.productID = itaddr->colValue;
            }
            if (itaddr->colName == "applyAmount")
            {
                orderInfo.applyAmount = itaddr->colValue;
            }
            if(itaddr->colName == "tradeType")
            {
                orderInfo.tradeType = S2I(itaddr->colValue);
            }
            if(itaddr->colName == "orderType")
            {
                orderInfo.orderType = S2I(itaddr->colValue);
            }
        }
    }

    return 0;
}

string Processor::getWalletOutOrderID(const long uid, const string& inOrderID)
{
    return "";
}

string Processor::getWalletChannelType(const string& inOrderID)
{
    return "";
}

long Processor::getWalletApplyAmount(const string& inOrderID)
{

    return 0;
}

int Processor::getWalletOrderStatus(const string& inOrderID)
{
    return -1;
}

string Processor::getWalletOrderPropsID(const string& inOrderID)
{
    return "";
}

long Processor::getWalletOrderClubID(const string& inOrderID)
{
    return -1;
}


long Processor::getWalletUID(const string& inOrderID)
{
    return 0;
}

int Processor::updateWalletOrder(const string& outOrderID, Json::Value dataJson)
{
    WalletOrderInfo orderInfo;
    selectWalletOrder(outOrderID, orderInfo);

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(WALLET_ORDER) + ":" + outOrderID;
    wdataReq.operateType = E_REDIS_WRITE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    wdataReq.clusterInfo.frageFactor = tars::hash<string>()(outOrderID);

    double dPayAmount = dataJson["amount"].asDouble();

    std::stringstream ss1;
    ss1 << std::setiosflags(std::ios::fixed) << std::setprecision(6) << dPayAmount;

    std::string sPayAmount = std::to_string(dPayAmount);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "amount";
    tfield.colType = STRING;
    tfield.colValue = sPayAmount;
    fields.push_back(tfield);
    tfield.colName = "status";
    tfield.colType = INT;
    tfield.colValue = I2S(dataJson["status"].asInt());//
    fields.push_back(tfield);
    tfield.colName = "payTime";
    tfield.colType = STRING;
    tfield.colValue = dataJson["payTime"].asString();
    fields.push_back(tfield);
    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "update wallet order err. iRet:" << iRet << ",wdataRsp:" << printTars(wdataRsp) << endl;
        return -1;
    }
    return 0;
}

int Processor::updateOrder(const map<string, string> &mapOrderInfo)
{
    auto it = mapOrderInfo.find("outOrderID");
    if(it == mapOrderInfo.end() || it->second.empty())
    {
        ROLLLOG_ERROR << "outOrderID is empty." << endl;
        return -1;
    }
    string outOrderID = it->second;

    WalletOrderInfo orderInfo;
    selectWalletOrder(outOrderID, orderInfo);

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(WALLET_ORDER) + ":" + outOrderID;
    wdataReq.operateType = E_REDIS_WRITE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    wdataReq.clusterInfo.frageFactor = tars::hash<string>()(outOrderID);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;

    for(auto item : mapOrderInfo)
    {
        if(item.first == "reviewer" || item.first == "auditTime" || item.first == "remark")
        {
            tfield.colName = item.first;
            tfield.colType = STRING;
            tfield.colValue = item.second;
        }
        else if(item.first == "status")
        {
            tfield.colName = item.first;
            tfield.colType = INT;
            tfield.colValue = item.second;
        }
        else
        {
            continue;
        }
        fields.push_back(tfield);
    }

    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "update wallet order err. iRet:" << iRet << ",wdataRsp:" << printTars(wdataRsp) << endl;
        return -3;
    }
    return 0;
}

void Processor::getProductGoodsGroup(const string&product_id, const long amount, orderProto::WalletOrderListResp_mRecordInfo* pRecordInfo)
{
    auto mall_config = g_app.getOuterFactoryPtr()->getMalllConfig();
    for(auto cfg : mall_config)
    {
        if(cfg["product_id"] != product_id)
        {
            continue;
        }

        string sExchange = cfg["exchangeGroup"] + (!cfg["extraGroup"].empty() ? "|" + cfg["extraGroup"] : "");

        map<int, long> mapExchange;

        //兑换组
        auto exchangeGroup = g_app.getOuterFactoryPtr()->split(sExchange, "|");
        for(auto item : exchangeGroup)
        {
            auto vecSub = g_app.getOuterFactoryPtr()->split(item, ":");
            if(vecSub.size() != 2)
            {
                continue;
            }
            auto it = mapExchange.find(S2I(vecSub[0]));
            if(it == mapExchange.end())
            {
                mapExchange.insert(std::make_pair(S2I(vecSub[0]), S2L(vecSub[1])));
            }
            else
            {
                mapExchange[S2I(vecSub[0])] += S2L(vecSub[1]);
            }
        }

        for(auto item : mapExchange)
        {
            auto ptr = pRecordInfo->add_goodsgroup();
            ptr->set_goodsid(item.first);

            long count = S2I(cfg["menuType"]) == 3 ? (item.first == 10000 ? amount : amount / 10) : item.second;
            ptr->set_goodscount(count);
        }
    }
}

bool Processor::checkPropsId(const string& product_id, const int props_id)
{
    auto mall_config = g_app.getOuterFactoryPtr()->getMalllConfig();
    for(auto cfg : mall_config)
    {
        if(cfg["product_id"] != product_id)
        {
            continue;
        }
        return S2I(cfg["props_id"]) == props_id;
    }
    return false;
}

//SELECT uid, t.total_count from tb_wallet_order as a, (select count(*) as recode_count from tb_wallet_order) as b where uid = '' limit 100, 100 
int Processor::selectWalletOrderListByUid(const long uid, const int page, const int days, const int props_id, orderProto::WalletOrderListResp &resp)
{
    string table_name = "tb_wallet_order";
    std::vector<string> col_name = {"uid"};

    std::vector<std::vector<string>> whlist = {
        {"uid","E_EQ" ,L2S(uid)},
    };

    if(days > 0)
    {
        whlist.push_back({"createTime","E_GE" ,g_app.getOuterFactoryPtr()->GetTimeDayLater(days)});
    }

    //查询详情
    dbagent::TDBReadRsp dataRsp;
    col_name = {"uid", "inOrderID", "tradeType", "toAddr", "status", "amount", "applyAmount", "productID" , "createTime"};
    int iRet = readDataFromDB(uid, table_name, col_name, whlist, "createTime", 0, 0, dataRsp);
    if(iRet != 0)
    {
        LOG_ERROR<<"select wallet order info err! uid: "<< uid << endl;
        return iRet;
    }

    vector<orderProto::WalletOrderListResp_mRecordInfo> vecRecordInfo;
    ROLLLOG_DEBUG << "size : " << dataRsp.records.size() << endl;
    for (auto it = dataRsp.records.begin(); it != dataRsp.records.end(); ++it)
    {
        string product_id = "0";
        orderProto::WalletOrderListResp_mRecordInfo recordInfo;
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if(itfield->colName == "createTime")
            {
                long time_second = g_app.getOuterFactoryPtr()->convertTimeStr2TimeStamp(itfield->colValue);
                recordInfo.set_optiontime(L2S(time_second));
            }
            else if(itfield->colName == "tradeType")
            {
                recordInfo.set_itradetype(S2I(itfield->colValue));
            }
            else if(itfield->colName == "toAddr")
            {
                recordInfo.set_address(S2I(itfield->colValue));
            }
            else if(itfield->colName == "inOrderID")
            {
                recordInfo.set_orderid(itfield->colValue);
            }
            else if (itfield->colName == "status")
            {
                recordInfo.set_istatus(S2I(itfield->colValue));
            }
            else if(itfield->colName == "amount")
            {
                recordInfo.set_lamount(S2I(itfield->colValue) * 100);

                recordInfo.set_samount(itfield->colValue);
            }
            else if(itfield->colName == "applyAmount")
            {
                recordInfo.set_lapplyamount(S2I(itfield->colValue) * 100);

                recordInfo.set_sapplyamount(itfield->colValue);
            }
            else if(itfield->colName == "productID")
            {
                product_id = itfield->colValue;
            }
        }
        if(true)
        {
            if(!product_id.empty())
            {
               getProductGoodsGroup(product_id, recordInfo.lamount(), &recordInfo);
            }
            vecRecordInfo.push_back(recordInfo);
        }
    }

    int startIndex = (int (page -1)) * g_app.getOuterFactoryPtr()->getLimitRecord();
    startIndex = startIndex < 0 ? 0 : startIndex;
    int endIndex = startIndex + g_app.getOuterFactoryPtr()->getLimitRecord();
    endIndex = endIndex > int(vecRecordInfo.size()) -1 ? int(vecRecordInfo.size()) -1 : endIndex;

    for(int i = startIndex; i <= endIndex; i++)
    {
        resp.add_record_list()->CopyFrom(vecRecordInfo[i]);
    }

    resp.set_record_count(vecRecordInfo.size());
    resp.set_show_count(g_app.getOuterFactoryPtr()->getLimitRecord());

    return 0;
}

//获取本日提现金额总数
int Processor::getDayWalletWithdrawAmount(const long uid)
{
    string table_name = "tb_wallet_order";
    std::vector<string> col_name = {"uid"};

    std::vector<std::vector<string>> whlist = {
        {"uid","E_EQ" ,L2S(uid)},
        {"status","E_EQ" ,"2"},
        {"tradeType","E_EQ" ,"2"},
        {"createTime","E_GE" ,g_app.getOuterFactoryPtr()->GetTimeDayZero()},
    };

    //查询详情
    dbagent::TDBReadRsp dataRsp;
    col_name = { "amount",};
    int iRet = readDataFromDB(uid, table_name, col_name, whlist, "createTime", 0, 0, dataRsp);
    if(iRet != 0)
    {
        LOG_ERROR<<"select wallet order info err! uid: "<< uid << endl;
        return 0;
    }

    long totalAmount = 0;
    for (auto it = dataRsp.records.begin(); it != dataRsp.records.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if(itfield->colName == "amount")
            {
                totalAmount += S2L(itfield->colValue);
            }
        }
    }

    return totalAmount;
}

//TG token
int Processor::insertBindTgInfo(const long uid, const string &strToken)
{
    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(TG_TOKEN) + ":" + strToken;
    wdataReq.operateType = E_REDIS_WRITE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    wdataReq.clusterInfo.frageFactor = tars::hash<string>()(strToken);

    TField tfield;
    tfield.colName = "uid";
    tfield.colType = BIGINT;
    tfield.colValue = L2S(uid);
    wdataReq.fields.push_back(tfield);

    TWriteDataRsp wdataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(wdataReq, wdataRsp);
    ROLLLOG_DEBUG << "set token data, iRet: " << iRet << ", wdataRsp: " << printTars(wdataRsp) << endl;
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "save token data err, iRet: " << iRet << ", iResult: " << wdataRsp.iResult << endl;
        return -1;
    }
    return 0;
}

long Processor::selectBindTgUidByToken(const string token)
{
    dataproxy::TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(TG_TOKEN) + ":" + token;
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(token);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "uid";
    fields.push_back(tfield);
    dataReq.fields = fields;

    TReadDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->redisRead(dataReq, dataRsp);
    ROLLLOG_DEBUG << "read token token data, iRet: " << iRet << ", datareq: " << printTars(dataReq) << ", dataRsp: " << printTars(dataRsp) << endl;
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "read token token err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -1;
    }

    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfields = it->begin(); itfields != it->end(); ++itfields)
        {
            if (itfields->colName == "uid")
            {
                return S2L(itfields->colValue);

            }
        }
    }
    return 0;
}

void Processor::deleteBindTgInfoByToken(const string token)
{
    dataproxy::TWriteDataRsp dataRsp;
    dataproxy::TWriteDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(TG_TOKEN) + ":" + token;
    dataReq.operateType = E_REDIS_DELETE;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(token);
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->redisWrite(dataReq, dataRsp);
    ROLLLOG_DEBUG << "delete token data, iRet: " << iRet << ", datareq: " << printTars(dataReq) << ", dataRsp: " << printTars(dataRsp) << endl;
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "delete token err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
    }
    return;
}

//获取本日提现金额总数
bool Processor::checkBindTG(const string &tgid)
{
    string table_name = "tb_useraccount";
    std::vector<string> col_name = {"uid"};

    std::vector<std::vector<string>> whlist = {
        {"bindTgId","E_EQ" ,tgid},
    };

    //查询详情
    dbagent::TDBReadRsp dataRsp;
    int iRet = readDataFromDB(0, table_name, col_name, whlist, "", 0, 0, dataRsp);
    if(iRet != 0)
    {
        LOG_ERROR<<"select user account tg bind info err! tgid: "<< tgid << endl;
        return false;
    }
    return dataRsp.records.size() != 0;
}
