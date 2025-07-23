#ifndef _Processor_H_
#define _Processor_H_

//
#include <util/tc_singleton.h>
#include "OrderServant.h"
#include "DataProxyProto.h"
#include <json/json.h>
#include "OuterFactoryImp.h"
#include "Order.pb.h"

//
using namespace tars;
using namespace std;
using namespace dataproxy;
using namespace dbagent;
/**
 *请求处理类
 *
 */
class Processor
{
public:
	/**
	 * 
	*/
	Processor();

	/**
	 * 
	*/
	~Processor();

public:
	//ORDER_IOS      = 87,  //tb_ios_order
	//查询
	int selectIOSOrder();
	//增加
	int UpdateIOSOrder();

	int readDataFromDB(long uid, const string& table_name, const std::vector<string>& col_name, const std::vector<std::vector<string>>& whlist, const string& order_col, int limit_num, int index,  dbagent::TDBReadRsp &dataRsp);

	int createWalletOrder(const string& outOrderID, WalletOrderInfo &orderInfo);

	int selectWalletOrder(const string& outOrderID, WalletOrderInfo &orderInfo);

	string getWalletOutOrderID(const long uid, const string& inOrderID);

	string getWalletChannelType(const string& inOrderID);

	long getWalletApplyAmount(const string& inOrderID);

	long getWalletUID(const string& inOrderID);

	int getWalletOrderStatus(const string& inOrderID);

	string getWalletOrderPropsID(const string& inOrderID);

	long getWalletOrderClubID(const string& inOrderID);

	int updateWalletOrder(const string& outOrderID, Json::Value dataJson);

	int updateOrder(const map<string, string> &mapOrderInfo);

	bool checkPropsId(const string& product_id, const int props_id);

	int selectWalletOrderListByUid(const long uid, const int page, const int days, const int props_id, orderProto::WalletOrderListResp &resp);

	void getProductGoodsGroup(const string& product_id, const long amount, orderProto::WalletOrderListResp_mRecordInfo* pRecordInfo);

	int getDayWalletWithdrawAmount(const long uid);

	int insertBindTgInfo(const long uid, const string &strToken);

	long selectBindTgUidByToken(const string token);

	void deleteBindTgInfoByToken(const string token);

	bool checkBindTG(const string &tgid);

};

//singleton
typedef TC_Singleton<Processor, CreateStatic, DefaultLifetime> ProcessorSingleton;

#endif

