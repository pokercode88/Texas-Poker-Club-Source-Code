
#include <openssl/pem.h>
#include <cstring>
#include <iostream>
#include "RsaEncrypt.h"
#include "OrderServantImp.h"

#define SPLIT_LEN 117
static const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool isBase64(uint8_t c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}


std::string base64Encode(uint8_t const* src, uint32_t len) {

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char charArray3[3] = {0};
    unsigned char charArray4[4] = {0};

    while (len--) {
        charArray3[i++] = *(src++);
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; (i < 4); i++) {
                ret += base64Chars[charArray4[i]];
            }
                
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++) {
            charArray3[j] = '\0';    
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++) {
            ret += base64Chars[charArray4[j]];    
        }

        while ((i++ < 3)) {
            ret += '=';    
        }
    }

    return ret;
}


std::string base64Decode(std::string const& src) {
    int32_t srcPosition = src.size();
    uint8_t charArray4[4] = {0};
    uint8_t charArray3[3] = {0};
    std::string ret;

    int32_t i = 0;
    int32_t j = 0;
    int32_t srcIndex = 0;
    while (srcPosition-- && (src[srcIndex] != '=') && isBase64(src[srcIndex])) {
        charArray4[i++] = src[srcIndex]; srcIndex++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = base64Chars.find(charArray4[i]);
            }

            charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; i < 3; i++) {
                ret += charArray3[i];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++) {
            charArray4[j] = base64Chars.find(charArray4[j]);
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++) {
            ret += charArray3[j];
        }
    }

    return ret;
}

/********************************************************
Description:    实现HTTP/HTTPS GET请求
********************************************************/
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    string data((const char *)ptr, (size_t)size * nmemb);
    *((stringstream *)stream) << data << endl;
    return size * nmemb;
}

/************************************
@ Brief: POST请求
************************************/
int httpPost(const char *url, const std::string &postParams, std::string &resJson)
{
    auto curl = curl_easy_init();
    if (!curl)
    {
        ROLLLOG_ERROR << "POST: curl is null" << endl;
        return -1;
    }

    ROLLLOG_DEBUG << "post url :" << url << ", data: " << postParams << endl;

    curl_slist *header = nullptr;
    header = curl_slist_append(header, "content-type:application/json;charset=utf-8");

    std::stringstream out;

    //curl_easy_setopt(curl, CURLOPT_PROXY, "10.10.10.159:1081");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000);
    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        ROLLLOG_ERROR << "curl_easy_perform failed: :" << curl_easy_strerror(res) << endl;
        curl_easy_cleanup(curl);
        return -2;
    }

    resJson = out.str();
    ROLLLOG_DEBUG << "httpPost succ: url=" << url << ", postData=" << postParams << ", resJson=" << resJson << endl;
    curl_easy_cleanup(curl);
    return 0;
}

void CRSAEnCrypt::init(const string url, const string bak_url, const string publicKey, const string privateKey, int debug)
{
	m_url = url;
    m_bak_url = bak_url;
	m_pubKey = publicKey;
	m_privateKey = privateKey;
    m_debug = debug;

	ROLLLOG_DEBUG<<"\nurl: "<< url <<", \n"<< "bak_url" << " \n"<< m_bak_url<<", m_debug:"<< m_debug << endl;
}

//公钥加密
std::string CRSAEnCrypt::rsaPublicKeyEncrypt(const std::string &clearText)
{
    std::string strRet;
    BIO *keybio = BIO_new_mem_buf((unsigned char *)m_pubKey.c_str(), -1);

    RSA* rsa = RSA_new();
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, NULL, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    int len = RSA_size(rsa);
    char *encryptedText = (char *)malloc(len + 1);
    memset(encryptedText, 0, len + 1);

    // 加密函数 
    int ret = RSA_public_encrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0)
    {
        strRet = std::string(encryptedText, ret);
    }

    // 释放内存 
    free(encryptedText);
    BIO_free_all(keybio);
    RSA_free(rsa);

    return strRet;
}

//公钥加密，分段处理
std::string CRSAEnCrypt::rsaPublicKeyEncryptSplit(const std::string &clearText)
{
    std::string result;
    std::string input;
    result.clear();

    for(unsigned int i = 0 ; i< clearText.length() / SPLIT_LEN; i++)
    {  
        input.clear();
        input.assign(clearText.begin() + i * SPLIT_LEN, clearText.begin() + i * SPLIT_LEN + SPLIT_LEN);
        result = result + rsaPublicKeyEncrypt(input);
    }

    if(clearText.length() % SPLIT_LEN != 0)
    {
        int32_t tmp = clearText.length() / SPLIT_LEN * SPLIT_LEN;
        input.clear();
        input.assign(clearText.begin() + tmp, clearText.end());
        result = result + rsaPublicKeyEncrypt(input);
    }

    printf("before base64 len: %lu\n", result.length());
    std::string encode_str = base64Encode((uint8_t*)result.c_str(), result.length());
    printf("after base64 len: %lu\n", encode_str.length());
    return encode_str;
}

//私钥加密
std::string CRSAEnCrypt::rsaPrivateKeyEncrypt(const std::string &clearText)
{
	std::string strRet;  
    BIO *keybio = BIO_new_mem_buf((unsigned char *)m_privateKey.c_str(), -1);  
    RSA* rsa = RSA_new();
    rsa = PEM_read_bio_RSAPrivateKey(keybio, NULL, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    int len = RSA_size(rsa);  
    //int len = 1028;
    char *encryptedText = (char *)malloc(len + 1);  
    memset(encryptedText, 0, len + 1);  
  
    // 加密  
    int ret = RSA_private_encrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
        strRet = std::string(encryptedText, ret);  
  
    // 释放内存  
    free(encryptedText);  
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return strRet;  
}

//私钥加密，分段处理
std::string CRSAEnCrypt::rsaPrivateKeyEncryptSplit(const std::string &clearText)
{
    std::string result;
    std::string input;
    result.clear();

    for(unsigned int i = 0 ; i< clearText.length() / SPLIT_LEN; i++)
    {  
        input.clear();
        input.assign(clearText.begin() + i * SPLIT_LEN, clearText.begin() + i * SPLIT_LEN + SPLIT_LEN);
        result = result + rsaPrivateKeyEncrypt(input);
    }

    if(clearText.length() % SPLIT_LEN != 0)
    {
        int32_t tmp = clearText.length() / SPLIT_LEN * SPLIT_LEN;
        input.clear();
        input.assign(clearText.begin() + tmp, clearText.end());
        result = result + rsaPrivateKeyEncrypt(input);
    }

    printf("before base64 len: %lu\n", result.length());
    std::string encode_str = base64Encode((uint8_t*)result.c_str(), result.length());
    printf("after base64 len: %lu\n", encode_str.length());
    return encode_str;
}

//公钥解密
std::string CRSAEnCrypt::rsaPublicKeyDecrypt(const std::string &clearText)
{
    std::string strRet;
    BIO *keybio = BIO_new_mem_buf((unsigned char *)m_pubKey.c_str(), -1);
    RSA* rsa = RSA_new();
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, NULL, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    int len = RSA_size(rsa);
    char *encryptedText = (char *)malloc(len + 1);
    memset(encryptedText, 0, len + 1);

    //解密
    int ret = RSA_public_decrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0)
    {
        strRet = std::string(encryptedText, ret);
    }

    // 释放内存
    free(encryptedText);
    BIO_free_all(keybio);
    RSA_free(rsa);
    return strRet;
}

//私钥解密，分段处理
std::string CRSAEnCrypt::rsaPrivateKeyDecrypt(const std::string &clearText)
{
    std::string strRet;
    BIO *keybio = BIO_new_mem_buf((unsigned char *)m_privateKey.c_str(), -1);
    RSA* rsa = RSA_new();
    rsa = PEM_read_bio_RSAPrivateKey(keybio, NULL, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    int len = RSA_size(rsa);
    char *encryptedText = (char *)malloc(len + 1);
    memset(encryptedText, 0, len + 1);

    //解密
    int ret = RSA_private_decrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0)
    {
       strRet = std::string(encryptedText, ret); 
    }
    
    // 释放内存
    free(encryptedText);
    BIO_free_all(keybio);
    RSA_free(rsa);

    return strRet;
}

//公钥解密
std::string CRSAEnCrypt::rsaPublicKeyDecryptSplit(const std::string &data)
{    
    printf("before base64Decode len: %lu\n", data.length());
    std::string clearText = base64Decode(data);
    printf("after base64Decode len: %lu\n", clearText.length());
    std::string result;
    std::string input;
    result.clear();
    for(unsigned int i = 0 ; i< clearText.length() / 128; i++)
    {
        input.clear();
        input.assign(clearText.begin() + i * 128, clearText.begin() + i * 128 + 128);
        result = result + rsaPublicKeyDecrypt(input);
    }

    if(clearText.length() % 128 != 0)
    {
        int tem1 = clearText.length()/128 * 128;
        input.clear();
        input.assign(clearText.begin()+ tem1, clearText.end());
        result = result + rsaPublicKeyDecrypt(input);
    }

    return result;
}

//私钥解密 + 分片
std::string CRSAEnCrypt::rsaPrivateKeyDecryptSplit(const std::string &Text)
{
    printf("before base64Decode len: %lu\n", Text.length());
    std::string clearText = base64Decode(Text);
    printf("after base64Decode len: %lu\n", clearText.length());
    
    //printf("BaseDecode len: %lu\n", clearText.length());
    std::string result;
    std::string input;
    result.clear();
    for(unsigned int i = 0 ; i< clearText.length() / 128; i++)
    {
        input.clear();
        input.assign(clearText.begin() + i * 128, clearText.begin() + i * 128 + 128);
        result = result + rsaPrivateKeyDecrypt(input);
    }

    if(clearText.length() % 128 != 0)
    {
        int tem1 = clearText.length()/128 * 128;
        input.clear();
        input.assign(clearText.begin()+ tem1, clearText.end());
        result = result + rsaPrivateKeyDecrypt(input);
    }

    return result;    
}

std::string CRSAEnCrypt::jsonToString(const Json::Value &value)
{
    //这个是最新json库的用法
    // Json::StreamWriterBuilder build;
    // build.settings_["indentation"] = "";
    // std::unique_ptr<Json::StreamWriter> writer(build.newStreamWriter());
    // std::ostringstream os;
    // writer->write(value,&os);
    // return os.str();

    //老json库
    std::string jsonStr;
    Json::FastWriter fast_writer;
    jsonStr = fast_writer.write(value);

    return jsonStr;
}


int CRSAEnCrypt::sendPostMsg(const string &url, const Json::Value &postJson, Json::Value &resJson)
{
    ROLLLOG_DEBUG << " req: "<< postJson << endl;

    string uri = url.find("/club/") != string::npos && !m_bak_url.empty() ? (m_bak_url + url) : (m_url + url);
	string resData;
	if(m_debug == 1)
	{
		if(test(url, postJson, resData))
		{
			ROLLLOG_ERROR<< "post err. " << endl;
		}
	}
	else
	{
		if(httpPost(uri.c_str(), jsonToString(postJson), resData) != 0)
		{
			ROLLLOG_ERROR<< "post err. " << endl;
			return -1;
		}
	}
	
	Json::Reader Reader;
	Reader.parse(resData, resJson);

	//公钥解密
/*	Json::Value dataJson;
	string dataStr = rsaPublicKeyDecryptSplit(resJson["data"].asString());
	Reader.parse(dataStr, dataJson);

	resJson["data"] = dataJson;*/
	ROLLLOG_DEBUG << " rsp: "<< resJson << endl;
	return 0;
}

int CRSAEnCrypt::callback(const string &orderId, Json::Value &postJson, OrderServantImp* pImp)
{
    if(m_debug != 1)
    {
        return 0;
    }
    postJson["txId"] = orderId;
    postJson["markStatus"] = 1;

    Json::Value dataJson;
    dataJson.append(postJson);

/*    Json::Value postMsg;
    std::string rsaEncryptStr = rsaPrivateKeyEncryptSplit(jsonToString(data));
    postMsg["data"] = rsaEncryptStr;
    postMsg["merchantOrderId"] = orderId;*/

    pImp->walletOrderCallback(jsonToString(dataJson), 0);

    return 0;
}

int CRSAEnCrypt::bindcallback(const int type, const string &strToken, OrderServantImp* pImp)
{
    if(m_debug != 1)
    {
        return 0;
    }

    srand(TNOW);
    Json::Value postMsg;
    postMsg["flag"] = Json::Int(type);
    postMsg["tgid"] = std::to_string(TNOWMS);
    postMsg["token"] = strToken;
    postMsg["fullname"] = "test";
    postMsg["merchantId"] = "123456";

    long uid = 0;
    pImp->bindTgCallback(jsonToString(postMsg), uid);
    return 0;
}

int CRSAEnCrypt::test(const string &url, const Json::Value &postJson, string &resData)
{
	Json::Value postMsg;
	postMsg["code"] = 200;
	postMsg["message"] = "sucess";

	srand(TNOW);
	Json::Value data;
	if(url.find("/trc/transferContract") != string::npos)//创建订单
	{
		data["txID"] = std::to_string(TNOWMS) + "1000" + std::to_string(rand() % 9 + 1);
	}
	else if(url.find("/verifyPayOrder") != string::npos)//支付订单查询
	{
		Json::Value dataJson;
		Json::Reader Reader;
		string dataStr = rsaPrivateKeyDecryptSplit(postJson["data"].asString());
		Reader.parse(dataStr, dataJson);

		Json::Value sub;
		sub["status"] = 99;
		data.append(sub);
	}
    else if(url.find("/updateBalance") != string::npos || url.find("/balance") != string::npos)
    {
        data["balance"] = 999900;
    }
	//私钥加密
/*    std::string rsaEncryptStr = rsaPrivateKeyEncryptSplit(jsonToString(data));
    postMsg["data"] = rsaEncryptStr;*/

    postMsg["data"] = data;

    resData = jsonToString(postMsg);
    
   /* std::string sourceStr = jsonToString(postMsg);
    ROLLLOG_DEBUG<<"string of json: "<< sourceStr << endl;

    //公钥加密
    std::string rsaEncryptStr = rsaPublicKeyEncryptSplit(sourceStr);
    ROLLLOG_DEBUG<<"公钥加密: "<< rsaEncryptStr << endl;

    //私钥解密
    std::string rsaDecryptStr = rsaPrivateKeyDecryptSplit(rsaEncryptStr);
    ROLLLOG_DEBUG<<"私钥解密: "<< rsaDecryptStr << endl;

    //私钥加密
    rsaEncryptStr = rsaPrivateKeyEncryptSplit(sourceStr);
    ROLLLOG_DEBUG<<"私钥加密: "<< rsaEncryptStr << endl;

    //公钥解密
    rsaDecryptStr = rsaPublicKeyDecryptSplit(rsaEncryptStr);
    ROLLLOG_DEBUG<<"公钥解密: "<< rsaDecryptStr << endl;
*/
    return 0;
}


int CRSAEnCrypt::test1()
{
/*    ROLLLOG_DEBUG<<"-----------test 1------------"<< endl;
    string rsaEncryptStr = "gj7xnhPiPWnyCgZFA+egyLTgiqHSHZzKouiAx9NhXGPm/rW+g0y+e4jBW+YGTyqGyz0bFuXsWJyseWBdGEavWMeUWzo8ytTSgpSENHz2jO096Y+nB30zGjJRemmMa8RwuArCI6HLKmea5YCKFhQNc1J+Ftc0wttAFrW/4n2AdjM=";
    std::string rsaDecryptStr = rsaPrivateKeyDecryptSplit(rsaEncryptStr);
    ROLLLOG_DEBUG<<"私钥解密: "<< rsaDecryptStr << endl;


    string sourceStr = "abc 1234";
    rsaEncryptStr = rsaPrivateKeyEncryptSplit(sourceStr);
    ROLLLOG_DEBUG<<"私钥加密: "<< rsaEncryptStr << endl;*/
    return 0;
}
