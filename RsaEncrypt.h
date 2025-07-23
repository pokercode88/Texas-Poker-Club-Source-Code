#ifndef _RSA_H_
#define _RSA_H_

#include <string>
//
#include <util/tc_singleton.h>
#include <json/json.h>
#include <curl/curl.h>
#include "OrderServant.h"
#include "OrderServantImp.h"
#include "LogComm.h"

//
using namespace tars;

class CRSAEnCrypt 
{
public:

	void init(const string url, const string bak_url, const string publicKey, const string privateKey, int debug);

	//公钥加密, 分段处理 
	//1024位，明文最大长度为1024/8 - 11 = 117
	std::string rsaPublicKeyEncryptSplit(const std::string &clearText);

	//私钥加密，分段处理
	std::string rsaPrivateKeyEncryptSplit(const std::string &clearText);

	//公钥解密，分段处理
	std::string rsaPublicKeyDecryptSplit(const std::string &clearText);

    //私钥解密，分段处理
    std::string rsaPrivateKeyDecryptSplit(const std::string &clearText);

    std::string jsonToString(const Json::Value &value);

    int sendPostMsg(const string &url, const Json::Value &postJson, Json::Value &resJson);

    int test(const string &url, const Json::Value &postJson, string &resData);

    int callback(const string &orderId, Json::Value &postJson, OrderServantImp* pImp);

    int bindcallback(const int type, const string &strToken, OrderServantImp* pImp);

	int test1();

private:
    //私钥解密
    std::string rsaPrivateKeyDecrypt(const std::string &clearText);

    //私钥加密
    std::string rsaPrivateKeyEncrypt(const std::string &clearText);

	//公钥加密
	std::string rsaPublicKeyEncrypt(const std::string &clearText);

	//公钥解密
	std::string rsaPublicKeyDecrypt(const std::string &clearText);

private:
    std::string m_privateKey;
	std::string m_pubKey;
	std::string m_url;
	std::string m_bak_url;
	int m_debug;

};

//singleton
typedef TC_Singleton<CRSAEnCrypt, CreateStatic, DefaultLifetime> CRSAEnCryptSingleton;

#endif
