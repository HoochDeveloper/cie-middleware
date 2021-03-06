#include "../stdafx.h"
#include "IAS.h"
#include "../crypto/ASNParser.h"
#include "../crypto/RSA.h"
#include "../crypto/AES.h"
#include "../crypto/SHA256.h"
#include "../crypto/SHA1.h"
#include "../crypto/DES3.h"
#include "../crypto/MAC.h"
#include <shlwapi.h>
#include <shlobj.h>
#include "../util/ModuleInfo.h"
#include "../res/resource.h"
#include "../../cacheLib/cacheLib.h"

#define CIE_KEY_DH_ID 0x81
#define CIE_KEY_ExtAuth_ID 0x84
#define CIE_PIN_ID 0x81
#define CIE_PUK_ID 0x82
#define CIE_KEY_Sign_ID 0x81

#define PP_AUTHCALLBACK 0xf0000001

// dllmain.h : Declaration of module class.

extern CModuleInfo moduleInfo;
extern ByteArray SkipZero(ByteArray &ba);

IAS::IAS(CToken::TokenTransmitCallback transmit,ByteArray ATR)
{
	Callback = nullptr;
	this->ATR = ATR;
	BYTE gemaltoAID[] = { 0xA0, 0x00, 0x00, 0x00, 0x30, 0x80, 0x00, 0x00, 0x00, 0x09, 0x81, 0x60, 0x01 };
	IAS_AID = VarToByteArray(gemaltoAID);

	BYTE AID[] = { 0xA0, 0x00, 0x00, 0x00, 0x00, 0x39 };
	CIE_AID = VarToByteArray(AID);
	
	token.setTransmitCallback(transmit, nullptr);
}


IAS::~IAS()
{
}


BYTE defModule[] = { 0xba, 0x28, 0x37, 0xab, 0x4c, 0x6b, 0xb8, 0x27, 0x57, 0x7b, 0xff, 0x4e, 0xb7, 0xb1, 0xe4, 0x9c, 0xdd, 0xe0, 0xf1, 0x66, 0x14, 0xd1, 0xef, 0x24, 0xc1, 0xb7, 0x5c, 0xf7, 0x0f, 0xb1, 0x2c, 0xd1, 0x8f, 0x4d, 0x14, 0xe2, 0x81, 0x4b, 0xa4, 0x87, 0x7e, 0xa8, 0x00, 0xe1, 0x75, 0x90, 0x60, 0x76, 0xb5, 0x62, 0xba, 0x53, 0x59, 0x73, 0xc5, 0xd8, 0xb3, 0x78, 0x05, 0x1d, 0x8a, 0xfc, 0x74, 0x07, 0xa1, 0xd9, 0x19, 0x52, 0x9e, 0x03, 0xc1, 0x06, 0xcd, 0xa1, 0x8d, 0x69, 0x9a, 0xfb, 0x0d, 0x8a, 0xb4, 0xfd, 0xdd, 0x9d, 0xc7, 0x19, 0x15, 0x9a, 0x50, 0xde, 0x94, 0x68, 0xf0, 0x2a, 0xb1, 0x03, 0xe2, 0x82, 0xa5, 0x0e, 0x71, 0x6e, 0xc2, 0x3c, 0xda, 0x5b, 0xfc, 0x4a, 0x23, 0x2b, 0x09, 0xa4, 0xb2, 0xc7, 0x07, 0x45, 0x93, 0x95, 0x49, 0x09, 0x9b, 0x44, 0x83, 0xcb, 0xae, 0x62, 0xd0, 0x09, 0x96, 0x74, 0xdb, 0xf6, 0xf3, 0x9b, 0x72, 0x23, 0xa9, 0x9d, 0x88, 0xe3, 0x3f, 0x1a, 0x0c, 0xde, 0xde, 0xeb, 0xbd, 0xc3, 0x55, 0x17, 0xab, 0xe9, 0x88, 0x0a, 0xab, 0x24, 0x0e, 0x1e, 0xa1, 0x66, 0x28, 0x3a, 0x27, 0x4a, 0x9a, 0xd9, 0x3b, 0x4b, 0x1d, 0x19, 0xf3, 0x67, 0x9f, 0x3e, 0x8b, 0x5f, 0xf6, 0xa1, 0xe0, 0xed, 0x73, 0x6e, 0x84, 0xd5, 0xab, 0xe0, 0x3c, 0x59, 0xe7, 0x34, 0x6b, 0x42, 0x18, 0x75, 0x5d, 0x75, 0x36, 0x6c, 0xbf, 0x41, 0x36, 0xf0, 0xa2, 0x6c, 0x3d, 0xc7, 0x0a, 0x69, 0xab, 0xaa, 0xf6, 0x6e, 0x13, 0xa1, 0xb2, 0xfa, 0xad, 0x05, 0x2c, 0xa6, 0xec, 0x9c, 0x51, 0xe2, 0xae, 0xd1, 0x4d, 0x16, 0xe0, 0x90, 0x25, 0x4d, 0xc3, 0xf6, 0x4e, 0xa2, 0xbd, 0x8a, 0x83, 0x6b, 0xba, 0x99, 0xde, 0xfa, 0xcb, 0xa3, 0xa6, 0x13, 0xae, 0xed, 0xd9, 0x3a, 0x96, 0x15, 0x27, 0x3d };
BYTE defPrivExp[] = { 0x47, 0x16, 0xc2, 0xa3, 0x8c, 0xcc, 0x7a, 0x07, 0xb4, 0x15, 0xeb, 0x1a, 0x61, 0x75, 0xf2, 0xaa, 0xa0, 0xe4, 0x9c, 0xea, 0xf1, 0xba, 0x75, 0xcb, 0xa0, 0x9a, 0x68, 0x4b, 0x04, 0xd8, 0x11, 0x18, 0x79, 0xd3, 0xe2, 0xcc, 0xd8, 0xb9, 0x4d, 0x3c, 0x5c, 0xf6, 0xc5, 0x57, 0x53, 0xf0, 0xed, 0x95, 0x87, 0x91, 0x0b, 0x3c, 0x77, 0x25, 0x8a, 0x01, 0x46, 0x0f, 0xe8, 0x4c, 0x2e, 0xde, 0x57, 0x64, 0xee, 0xbe, 0x9c, 0x37, 0xfb, 0x95, 0xcd, 0x69, 0xce, 0xaf, 0x09, 0xf4, 0xb1, 0x35, 0x7c, 0x27, 0x63, 0x14, 0xab, 0x43, 0xec, 0x5b, 0x3c, 0xef, 0xb0, 0x40, 0x3f, 0x86, 0x8f, 0x68, 0x8e, 0x2e, 0xc0, 0x9a, 0x49, 0x73, 0xe9, 0x87, 0x75, 0x6f, 0x8d, 0xa7, 0xa1, 0x01, 0xa2, 0xca, 0x75, 0xa5, 0x4a, 0x8c, 0x4c, 0xcf, 0x9a, 0x1b, 0x61, 0x47, 0xe4, 0xde, 0x56, 0x42, 0x3a, 0xf7, 0x0b, 0x20, 0x67, 0x17, 0x9c, 0x5e, 0xeb, 0x64, 0x68, 0x67, 0x86, 0x34, 0x78, 0xd7, 0x52, 0xc7, 0xf4, 0x12, 0xdb, 0x27, 0x75, 0x41, 0x57, 0x5a, 0xa0, 0x61, 0x9d, 0x30, 0xbc, 0xcc, 0x8d, 0x87, 0xe6, 0x17, 0x0b, 0x33, 0x43, 0x9a, 0x2c, 0x93, 0xf2, 0xd9, 0x7e, 0x18, 0xc0, 0xa8, 0x23, 0x43, 0xa6, 0x01, 0x2a, 0x5b, 0xb1, 0x82, 0x28, 0x08, 0xf0, 0x1b, 0x5c, 0xfd, 0x85, 0x67, 0x3a, 0xc0, 0x96, 0x4c, 0x5f, 0x3c, 0xfd, 0x2d, 0xaf, 0x81, 0x42, 0x35, 0x97, 0x64, 0xa9, 0xad, 0xb9, 0xe3, 0xf7, 0x6d, 0xb6, 0x13, 0x46, 0x1c, 0x1b, 0xc9, 0x13, 0xdc, 0x9a, 0xc0, 0xab, 0x50, 0xd3, 0x65, 0xf7, 0x7c, 0xb9, 0x31, 0x94, 0xc9, 0x8a, 0xa9, 0x66, 0xd8, 0x9c, 0xdd, 0x55, 0x51, 0x25, 0xa5, 0xe5, 0x9e, 0xcf, 0x4f, 0xa3, 0xf0, 0xc3, 0xfd, 0x61, 0x0c, 0xd3, 0xd0, 0x56, 0x43, 0x93, 0x38, 0xfd, 0x81 };
BYTE defPubExp[] = { 0x00, 0x01, 0x00, 0x01 };

void IAS::ReadSOD(ByteDynArray &data) {
	init_func
	SelectAID_CIE();
	readfile(0x1006,data);
	exit_func
}
void IAS::ReadDH(ByteDynArray &data) {
	init_func
	SelectAID_IAS();
	readfile(0xd004, data);
	exit_func
}
void IAS::ReadCertCIE(ByteDynArray &data) {
	init_func
	SelectAID_CIE(true);
	readfile_SM(0x1003, data);
	exit_func
}
void IAS::ReadServiziPubKey(ByteDynArray &data) {
	init_func
	SelectAID_CIE();
	readfile(0x1005, data);
	exit_func
}
void IAS::ReadSerialeCIE(ByteDynArray &data) {
	init_func
	SelectAID_CIE(true);
	readfile_SM(0x1002, data);
	exit_func
}
void IAS::ReadIdServizi(ByteDynArray &data) {
	init_func
	SelectAID_CIE();
	readfile(0x1001, data);
	exit_func
}

void IAS::Sign(ByteArray &data, ByteDynArray &signedData) {
	init_func
	ByteDynArray resp;
	BYTE SetKey[] = { 0x00, 0x22, 0x41, 0xA4 };
	BYTE val02 = 2;
	BYTE keyId = CIE_KEY_Sign_ID;
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(SetKey), ASN1Tag(0x80, VarToByteArray(val02)).append(ASN1Tag(0x84, VarToByteArray(keyId))), resp))
	
	BYTE Sign[] = { 0x00, 0x88, 0x00, 0x00 };
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(Sign), data, signedData))
	exit_func
}

DWORD IAS::VerifyPUK(ByteArray &PIN) {
	init_func
	ByteDynArray resp;
	BYTE verifyPIN[] = { 0x00, 0x20, 0x00, CIE_PUK_ID };
	return SendAPDU_SM(VarToByteArray(verifyPIN), PIN, resp);
	exit_func
}

DWORD IAS::VerifyPIN(ByteArray &PIN) {
	init_func
	ByteDynArray resp;
	BYTE verifyPIN[] = { 0x00, 0x20, 0x00, CIE_PIN_ID };
	return SendAPDU_SM(VarToByteArray(verifyPIN), PIN, resp);
	exit_func
}

DWORD IAS::UnblockPIN() {
	init_func
	ByteDynArray resp;
	BYTE UnblockPIN[] = { 0x00, 0x2C, 0x03, CIE_PIN_ID };
	return SendAPDU_SM(VarToByteArray(UnblockPIN), ByteArray(), resp);
	exit_func
}

DWORD IAS::ChangePIN(ByteArray &oldPIN,ByteArray &newPIN) {
	init_func
	ByteDynArray resp;
	ByteDynArray data=oldPIN;
	data.append(newPIN);
	BYTE ChangePIN[] = { 0x00, 0x24, 0x00, CIE_PIN_ID };
	return SendAPDU_SM(VarToByteArray(ChangePIN), data, resp);
	exit_func
}

DWORD IAS::ChangePIN(ByteArray &newPIN) {
	init_func
	ByteDynArray resp;
	BYTE ChangePIN[] = { 0x00, 0x2C, 0x02, CIE_PIN_ID };
	return SendAPDU_SM(VarToByteArray(ChangePIN), newPIN, resp);
	exit_func
}

void IAS::readfile(WORD id, ByteDynArray &content){
	init_func

	ByteDynArray resp;
	BYTE selectFile[] = { 0x00, 0xa4, 0x02, 0x04 };
	BYTE fileId[] = { id >> 8, id & 0xff };
	CARD_R_CALL(SendAPDU(VarToByteArray(selectFile), VarToByteArray(fileId), resp))


	int cnt = 0;
	BYTE chunk = 128;
	DWORD sw;
	while (true) {
		ByteDynArray chn;
		BYTE readFile[] = { 0x00, 0xb0, cnt >> 8, cnt & 0xff };
		sw = SendAPDU(VarToByteArray(readFile), ByteArray(), chn, &chunk);
		if ((sw >> 8) == 0x6c)  {
			BYTE le = sw & 0xff;
			sw = SendAPDU(VarToByteArray(readFile), ByteArray(), chn, &le);
		}
		if (sw == 0x9000) {
			content.append(chn);
			cnt += chn.size();
			chunk = 128;
		}
		else {
			if (sw == 0x6282)
				content.append(chn);
			else if (sw != 0x6b00)
				throw CSCardException((WORD)sw);
			break;
		}
	}
	exit_func
}

void IAS::readfile_SM(WORD id, ByteDynArray &content) {
	init_func

	ByteDynArray resp;
	BYTE selectFile[] = { 0x00, 0xa4, 0x02, 0x04 };
	BYTE fileId[] = { id >> 8, id & 0xff };
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(selectFile), VarToByteArray(fileId), resp))


	int cnt = 0;
	BYTE chunk = 128;
	DWORD sw;
	while (true) {
		ByteDynArray chn;
		BYTE readFile[] = { 0x00, 0xb0, cnt >> 8, cnt & 0xff };
		sw = SendAPDU_SM(VarToByteArray(readFile), ByteArray(), chn, &chunk);
		if ((sw >> 8) == 0x6c)  {
			BYTE le = sw & 0xff;
			sw = SendAPDU_SM(VarToByteArray(readFile), ByteArray(), chn, &le);
		}
		if (sw == 0x9000) {
			content.append(chn);
			cnt += chn.size();
			chunk = 128;
		}
		else {
			if (sw == 0x6282) 
				content.append(chn);
			else if (sw != 0x6b00)
				throw CSCardException((WORD)sw);
			break;
		}
	}
	exit_func
}

void IAS::SelectAID_CIE(bool SM) {
	init_func
	ByteDynArray resp;
	BYTE selectCIE[] = { 0x00, 0xa4, 0x04, 0x0c };
	ByteDynArray selectCIEapdu;
	if (SM) {
		CARD_R_CALL(SendAPDU_SM(VarToByteArray(selectCIE), CIE_AID, resp))
	}
	else
	{
		CARD_R_CALL(SendAPDU(VarToByteArray(selectCIE), CIE_AID, resp))
	}
	exit_func
}

BYTE NXP_ATR[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
BYTE Gemalto_ATR[] = { 0x80, 0x31, 0x80, 0x65, 0xB0, 0x85, 0x04, 0x00, 0x11, 0x12, 0x0F, 0xFF, 0x82, 0x90, 0x00 };
BYTE Gemalto2_ATR[] = {0x80, 0x31, 0x80, 0x65, 0xB0, 0x85, 0x03, 0x00, 0xEF, 0x12, 0x0F, 0xFF, 0x82, 0x90, 0x00 };
ByteArray baNXP_ATR(NXP_ATR, sizeof(NXP_ATR));
ByteArray baGemalto_ATR(Gemalto_ATR, sizeof(Gemalto_ATR));
ByteArray baGemalto2_ATR(Gemalto2_ATR, sizeof(Gemalto2_ATR));

void IAS::ReadCIEType() {	
	if (ATR.IndexOf(baNXP_ATR) >= 0)
		type = CIE_Type::CIE_NXP;
	else if (ATR.IndexOf(baGemalto_ATR) >= 0)
		type = CIE_Type::CIE_Gemalto;
	else if (ATR.IndexOf(baGemalto2_ATR) >= 0)
		type = CIE_Type::CIE_Gemalto;
	else
		throw CStringException("CIE non riconosciuta");
}

void IAS::SelectAID_IAS(bool SM) {
	init_func
	if (type == CIE_Type::CIE_Unknown) {
		ReadCIEType();
	}
	ByteDynArray resp;
	if (type != CIE_Type::CIE_NXP)  {
		BYTE selectIAS[] = { 0x00, 0xa4, 0x04, 0x0c };
		if (SM)
		{
			CARD_R_CALL(SendAPDU_SM(VarToByteArray(selectIAS), IAS_AID, resp))
		}
		else
		{
			CARD_R_CALL(SendAPDU(VarToByteArray(selectIAS), IAS_AID, resp))
		}
	}
	else {
		BYTE selectMF[] = { 0x00, 0xa4, 0x00, 0x00 };
		if (SM)
		{
			CARD_R_CALL(SendAPDU_SM(VarToByteArray(selectMF), ByteArray(), resp))
		}
		else
		{
			CARD_R_CALL(SendAPDU(VarToByteArray(selectMF), ByteArray(), resp))
		}
	}
	exit_func
}

void IAS::ReadDappPubKey(ByteDynArray &DappKey) {
	init_func
	ByteDynArray resp;
	SelectAID_CIE();
	readfile(0x1004, DappKey);

	CASNParser parser;
	parser.Parse(DappKey);
	DappModule = parser.tags[0]->tags[0]->content;
	while (DappModule[0] == 0)
		DappModule = DappModule.mid(1);
	DappPubKey = parser.tags[0]->tags[1]->content;
	while (DappPubKey[0] == 0)
		DappPubKey = DappPubKey.mid(1);
	exit_func
}

void IAS::ReadPAN() {
	init_func
	SelectAID_IAS();
	readfile(0xd003, PAN);

	exit_func
}

void IAS::DAPP() {
	init_func
	ByteDynArray resp;
	BYTE psoVerifyAlgo = 0x41;
	BYTE PKdScheme = 0x9B;
	BYTE shaOID = 0x04;
	DWORD shaSize = 32;
	CSHA256 sha256;
	BYTE Val01 = 1;

	if (DappPubKey.isEmpty()) {
		throw CStringException("La chiave DAPP non � diponibile");
	}

	ByteDynArray module = VarToByteArray(defModule);
	ByteDynArray pubexp = VarToByteArray(defPubExp);
	ByteDynArray privexp = VarToByteArray(defPrivExp);

	ByteDynArray cert;
	ByteDynArray CHR, CHA, OID;


	BYTE snIFD[] = { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
	BYTE CPI=0x8A;
	BYTE baseCHR[] = { 0x00, 0x00, 0x00, 0x00 };
	CHR.set(2, &VarToByteArray(baseCHR), &VarToByteArray(snIFD));
	CHA.set(2,&CA_AID,01);
	BYTE baseOID[] = { 0x2A, 0x81, 0x22, 0xF4, 0x2A, 0x02, 0x04, 0x01 };
	OID.set(2, &VarToByteArray(baseOID), shaOID);

	ByteDynArray endEntityCert;
	endEntityCert.set(0, CPI, &CA_CAR, &CHR, &CHA, &OID, &module, &pubexp, &term___set);

	ByteDynArray certSign, toSign, d1;
	sha256.Digest(endEntityCert, d1);
	BYTE ValBC = 0xBC;
	toSign.set(0, 0x6A, &(endEntityCert.left(CA_module.size() - shaSize - 2)), &d1, 0xbc, &term___set);
	CRSA caKey(CA_module, CA_privexp);
	caKey.RSA_PURE(toSign, certSign);
	ByteDynArray certVerif;
	CRSA caPubKey(CA_module, CA_pubexp);
	caPubKey.RSA_PURE(certSign, certVerif);
	ER_ASSERT(certVerif == toSign, "Errore in verifica firma!")

	ByteDynArray PkRem;
	PkRem = endEntityCert.mid(CA_module.size() - shaSize - 2);

	cert.setASN1Tag(0x7F21, ASN1Tag(0x5F37, certSign).append(ASN1Tag(0x5F38, PkRem)).append(ASN1Tag(0x42, CA_CAR)));

	BYTE SelectKey[] = { 0x00, 0x22, 0x81, 0xb6 };
	BYTE id = CIE_KEY_ExtAuth_ID;
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(SelectKey), ASN1Tag(0x80, VarToByteArray(psoVerifyAlgo)).append(ASN1Tag(0x83, VarToByteArray(id))), resp))

	BYTE VerifyCert[] = { 0x00, 0x2A, 0x00, 0xAE };
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(VerifyCert), cert, resp))

	BYTE SetCHR[] = { 0x00, 0x22, 0x81, 0xA4 };
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(SetCHR), ASN1Tag(0x83, CHR), resp))

	ByteDynArray challenge;
	BYTE GetChallenge[] = { 0x00, 0x84, 0x00, 0x00 };
	BYTE chLen = 8;

	CARD_R_CALL(SendAPDU_SM(VarToByteArray(GetChallenge), ByteArray(), challenge, &chLen))

	ByteDynArray toHash;
	DWORD padSize = module.size() - shaSize - 2;
	ByteDynArray PRND;
	PRND.random(padSize);
	toHash.set(0, &PRND, &dh_pubKey, &VarToByteArray(snIFD), &challenge, &dh_ICCpubKey, &dh_g, &dh_p, &dh_q, &term___set);
	sha256.Digest(toHash,d1);
	toSign.set(0, 0x6a, &PRND, &d1, 0xBC, &term___set);
	ByteDynArray signResp;
	CRSA certKey(module, privexp);
	certKey.RSA_PURE(toSign, signResp);
	ByteDynArray chResponse;
	chResponse.set(2, &VarToByteArray(snIFD), &signResp);

	BYTE ExtAuth[] = { 0x00, 0x82, 0x00, 0x00 };
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(ExtAuth), chResponse, resp))


	BYTE IntAuth[] = { 0x00, 0x22, 0x41, 0xa4 };
	BYTE Val82 = 0x82;
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(IntAuth), ASN1Tag(0x84, VarToByteArray(Val82)).append(ASN1Tag(0x80, VarToByteArray(PKdScheme))), resp))

	ByteDynArray rndIFD;
	rndIFD.random(8);
	BYTE GiveRandom[] = { 0x00, 0x88, 0x00, 0x00 };
	CARD_R_CALL(SendAPDU_SM(VarToByteArray(GiveRandom), rndIFD, resp, 0))

	ByteDynArray SN_ICC = resp.mid(0, 8);
	ByteDynArray intAuthResp;
	CRSA intAuthKey(DappModule, DappPubKey);
	intAuthKey.RSA_PURE(resp.mid(8), intAuthResp);
	ER_ASSERT(intAuthResp[0] == 0x6a, "Errore nell'autenticazione del chip");
	ByteArray PRND2 = intAuthResp.mid(1, intAuthResp.size() - 32 - 2);
	ByteArray hashICC = intAuthResp.mid(PRND2.size() + 1, 32);

	ByteDynArray toHashIFD, calcHashIFD;
	toHashIFD.set(0, &PRND2, &dh_ICCpubKey, &SN_ICC, &rndIFD, &dh_pubKey, &dh_g, &dh_p, &dh_q, &term___set);
	sha256.Digest(toHashIFD, calcHashIFD);
	ER_ASSERT(calcHashIFD == hashICC, "Errore nell'autenticazione del chip")
	ER_ASSERT(intAuthResp.right(1)[0] == 0xbc, "Errore nell'autenticazione del chip");

	sessSSC.set(2, &(challenge.right(4)), &(rndIFD.right(4)), &term___set);
	exit_func
}

void IAS::DHKeyExchange() {
	init_func
	CASNParser asn1;
	ByteDynArray dh_prKey, secret, resp,d1;
	do {
		dh_prKey.random(dh_q.size());
	} while (dh_q[0] < dh_prKey[0]);


	ByteDynArray dhg(dh_g.size());
	dhg.fill(0);
	dhg.rightcopy(dh_g);
	CRSA rsa(dh_p, dh_prKey);
	rsa.RSA_PURE(dhg, dh_pubKey);

	BYTE algo = 0x9b;
	BYTE keyId = CIE_KEY_DH_ID;
	d1.setASN1Tag(0x80, VarToByteArray(algo)).append(ASN1Tag(0x83, VarToByteArray(keyId))).append(ASN1Tag(0x91, dh_pubKey));

	BYTE MSE_SET[] = { 0x00, 0x22, 0x41, 0xa6 };
	CARD_R_CALL(SendAPDU(VarToByteArray(MSE_SET), d1, resp))
	BYTE GET_DATA[] = { 0x00, 0xcb, 0x3f, 0xff };
	BYTE GET_DATA_Data[] = { 0x4d, 0x04, 0xa6, 0x02, 0x91, 0x00 };
	CARD_R_CALL(SendAPDU(VarToByteArray(GET_DATA), VarToByteArray(GET_DATA_Data), resp))

	asn1.Parse(resp);
	dh_ICCpubKey = asn1.tags[0]->tags[0]->content;

	rsa.RSA_PURE(dh_ICCpubKey, secret);

	CSHA256 sha256;
	
	BYTE diffENC[] = { 0x00, 0x00, 0x00, 0x01 };
	BYTE diffMAC[] = { 0x00, 0x00, 0x00, 0x02 };
	sha256.Digest(ByteDynArray(secret).append(VarToByteArray(diffENC)), d1); sessENC = d1.left(16);
	sha256.Digest(ByteDynArray(secret).append(VarToByteArray(diffMAC)), d1); sessMAC = d1.left(16);
	sessSSC.resize(8); 
	sessSSC.fill(0);
	sessSSC[7] = 1;
	exit_func
}

void IAS::increment(ByteArray &seq) {
	for (int i = seq.size() - 1; i >= 0; i--) {
		if (seq[i] < 255) {
			seq[i]++;
			for (DWORD j = i + 1; j < seq.size(); j++)
				seq[j] = 0;
			return;
		}
	}
}


DWORD IAS::SM(ByteArray &keyEnc, ByteArray &keySig, ByteArray &apdu, ByteArray &seq, ByteDynArray &elabResp) {
	init_func

	increment(seq);
	ByteDynArray calcMac, smMac, smHead, d1;
	smHead = apdu.left(4);
	smHead[0] |= 0x0C;
	ISOPad(ByteDynArray(seq).append(smHead), calcMac);
	CDES3 encDes(keyEnc);
	CMAC sigMac(keySig);
	BYTE Val01 = 1;
	BYTE Val00 = 0;

	ByteDynArray datafield, doob;
	if (apdu[4] != 0 && apdu.size() > 5) {
		ByteDynArray enc;
		ISOPad(apdu.mid(5, apdu[4]), d1);
		encDes.RawEncode(d1, enc);
		if ((apdu[1] & 1) == 0)
			doob.setASN1Tag(0x87, VarToByteDynArray(Val01).append(enc));
		else
			doob.setASN1Tag(0x85, enc);

		calcMac.append(doob);
		datafield.append(doob);
	}
	if (apdu.size() == 5 || apdu.size() == (apdu[4] + 6)) {
		BYTE le = apdu[apdu.size() - 1];
		doob.setASN1Tag(0x97, VarToByteArray(le));
		calcMac.append(doob);
		datafield.append(doob);
	}
	ISOPad(calcMac, d1);
	sigMac.Mac(d1, smMac);
	datafield.append(ASN1Tag(0x8e, smMac));

	elabResp.set(0, &smHead, datafield.size(), &datafield, 0x00, &term___set);
	return OK;
	exit_func
}

DWORD IAS::respSM(ByteArray &keyEnc, ByteArray &keySig, ByteArray &resp, ByteArray &seq, ByteDynArray &elabResp) {
	init_func

	increment(seq);
	DWORD index, llen, lgn, sw=0xffff;
	ByteDynArray encData;
	ByteDynArray respMac, calcMac;
	CDES3 encDes(keyEnc);
	CMAC sigMac(keySig);

	calcMac = seq;
	index = 0;
	do {

		if (resp[index] == 0x99) {
			calcMac.append(resp.mid(index, resp[index + 1] + 2));
			sw = resp[index + 2] << 8 | resp[index + 3];
			index += 4;
		}
		else if (resp[index] == 0x8e) {
			if (resp[index + 1] != 0x08)
				throw CStringException("Lunghezza del MAC non valida");
			respMac = resp.mid(index + 2, 8);
			index += 10;
		}
		else if (resp[index] == 0x85) {
			if (resp[index + 1] > 0x80) {
				llen = resp[index + 1] - 0x80;
				if (llen == 1) 
					lgn = resp[index + 2];
				else 
					if (llen == 2) 
						lgn = (resp[index + 2] << 8) | resp[index + 3];
					else 
						throw CStringException("Lunghezza ASN1 non valida: %i", llen);
				encData = resp.mid(index + llen + 2, lgn);
				calcMac.append(resp.mid(index, lgn + llen + 2));
				index += llen + lgn + 2;
			}
			else {
				encData = resp.mid(index + 2, resp[index + 1]);
				calcMac.append(resp.mid(index, resp[index + 1] + 2));
				index += resp[index + 1] + 2;
			}
		}
		else if (resp[index] == 0x87) {
			if (resp[index + 1] > 0x80) {
				llen = resp[index + 1] - 0x80;
				if (llen == 1) 
					lgn = resp[index + 2];
				else 
					if (llen == 2) 
						lgn = (resp[index + 2] << 8) | resp[index + 3];
					else
						throw CStringException("Lunghezza ASN1 non valida: %i", llen);
				encData = resp.mid(index + llen + 3, lgn - 1);
				calcMac.append(resp.mid(index, lgn + llen + 2));
				index += llen + lgn + 2;
			}
			else {
				encData = resp.mid(index + 3, resp[index + 1] - 1);
				calcMac.append(resp.mid(index, resp[index + 1] + 2));
				index += resp[index + 1] + 2;
			}
		}
		else
			throw CStringException("Tag non riconosciuto nella risposta in Secure Messaging");
	} while (index < resp.size());

	ByteDynArray smMac,d1;
	ISOPad(calcMac, d1);
	sigMac.Mac(d1, smMac);
	ER_ASSERT(smMac == respMac,"Errore nel checksum della risposta del chip")

	if (!encData.isEmpty()) {
		encDes.RawDecode(encData, elabResp);
		elabResp.resize(RemoveISOPad(elabResp),true);
	}
	else
		elabResp.clear();

	return sw;
	exit_func
}

DWORD IAS::getResp(ByteDynArray &resp, DWORD sw,ByteDynArray &elabresp) {
	init_func
	elabresp.clear();
	if (resp.size() != 0)
		elabresp.append(resp);
		
	ByteDynArray curresp;
	while (true) {
		if ((sw >> 8) == 0x61) {
			BYTE ln = sw & 0xff;
			if (ln != 0) {
				BYTE apdu[] = { 0x00, 0xc0, 0x00, 0x00, ln };
				sw = token.Transmit(VarToByteArray(apdu), &curresp);
				elabresp.append(curresp);
				return sw;
			}
			else {
				BYTE apdu[] = { 0x00, 0xc0, 0x00, 0x00, 0x00 };
				sw = token.Transmit(VarToByteArray(apdu), &curresp);
				elabresp.append(curresp);
			}
		}
		else {
			return sw;
		}
	}
	exit_func
}

DWORD IAS::getResp_SM(ByteArray &resp, DWORD sw, ByteDynArray &elabresp) {
	init_func

	
	ByteDynArray s, ap;
	CASNParser p;
	elabresp.clear();
	if (resp.size() != 0)
		elabresp.append(resp);

	ByteDynArray curresp;
	while (true) {
		if ((sw >> 8) == 0x61) {
			BYTE ln = sw & 0xff;
			if (ln != 0) {
				BYTE apdu[] = { 0x00, 0xc0, 0x00, 0x00, ln };
				sw = token.Transmit(VarToByteArray(apdu), &curresp);
				elabresp.append(curresp);
				if (sw == 0x9000)
					break;
				if ((sw >> 8) != 0x61)
					CARD_R_CALL(sw);
			}
			else {
				BYTE apdu[] = { 0x00, 0xc0, 0x00, 0x00, 0x00 };
				sw = token.Transmit(VarToByteArray(apdu), &curresp);
				elabresp.append(curresp);
			}
		}
		else  if (sw == 0x9000 || sw == 0x6b00 || sw==0x6282)
			break;
		else
			return sw;
	}
	return respSM(sessENC, sessMAC, elabresp, sessSSC, elabresp);
	exit_func
}


DWORD IAS::SendAPDU_SM(ByteArray &head, ByteArray &data, ByteDynArray &resp, BYTE *le) {
	init_func
	ByteDynArray smApdu;
	ByteDynArray s, curresp;
	DWORD sw;
	if (data.size() < 0xE7) {
		smApdu.set(0, &head, data.size(), &data, le == nullptr ? &(ByteArray()) : &(VarToByteArray(*le)), &term___set);

		ODS(String().printf("Clear APDU: %s\n", dumpHexData(smApdu, String()).lock()).lock());
		SM(sessENC, sessMAC, smApdu, sessSSC, smApdu);
		sw = token.Transmit(smApdu, &curresp);
		sw = getResp_SM(curresp, sw, resp);

		ODS(String().printf("Clear RESP: %s %02X %02X\n", dumpHexData(resp, String()).lock(), sw >> 8, sw & 0xff).lock());
		return sw;
	}
	else {
		DWORD ds = data.size();
		DWORD i = 0;
		BYTE cla = head[0];
		while (true) {
			s = data.mid(i, min(0xE7, data.size() - i));
			i += s.size();
			if (i != data.size())
				head[0] = cla | 0x10;
			else
				head[0] = cla;
			if (s.size()!=0)
				smApdu.set(4, &head, (BYTE)s.size(), &s, (le == nullptr || i<data.size()) ? &(ByteArray()) : &(VarToByteArray(*le)));
			else
				smApdu.set(2, &head, (le == nullptr || i<data.size()) ? &(ByteArray()) : &(VarToByteArray(*le)));
			ODS(String().printf("Clear APDU: %s\n", dumpHexData(smApdu, String()).lock()).lock());
			SM(sessENC, sessMAC, smApdu, sessSSC, smApdu);
			sw = token.Transmit(smApdu, &curresp);
			sw = getResp_SM(curresp, sw, resp);

			ODS(String().printf("Clear RESP: %s %02X %02X\n", dumpHexData(resp, String()).lock(), sw >> 8, sw & 0xff).lock());
			if (i == data.size())
				return sw;
		}
	}
	exit_func
}


DWORD IAS::SendAPDU(ByteArray &head, ByteArray &data, ByteDynArray &resp, BYTE *le) {
	init_func

	ByteDynArray apdu, curresp;
	auto ds = data.size();

	if (ds > 255) {
		DWORD i = 0;
		BYTE cla = head[0];
		while (true) {
			auto s = data.mid(i, min(data.size()-i,255));
			i += s.size();
			if (i != data.size())
				head[0] = (cla | 0x10);
			else
				head[0] = cla;

			apdu.set(4, &head, (BYTE)s.size(), &s, le == nullptr ? &(ByteArray()) : &(VarToByteArray(*le)));

			DWORD sw=token.Transmit(apdu, &curresp);
			if (i == data.size()) {
				sw = getResp(curresp, sw, resp);

				return sw;
			}
		}
	}
	else {
		if (data.size()!=0)
			apdu.set(4, &head, (BYTE)data.size(), &data, le == nullptr ? &(ByteArray()) : &(VarToByteArray(*le)));
		else
			apdu.set(2, &head, le == nullptr ? &(ByteArray()) : &(VarToByteArray(*le)));

		DWORD sw = token.Transmit(apdu, &curresp);
		sw=getResp(curresp, sw, resp);

		return sw;
	}
	exit_func
}


void IAS::InitDHParam() {
	init_func
	ByteDynArray resp;
	SelectAID_CIE();

	CASNParser parser;
	if (type == CIE_Type::CIE_Gemalto) {
		BYTE getDHDoup[] = { 00, 0xcb, 0x3f, 0xff };
		BYTE getDHDuopData[] = { 0x4d, 0x08, 0x70, 0x06, 0xBF, 0xA1, 0x01, 0x02, 0xA3, 0x80 };
		CARD_R_CALL(SendAPDU(VarToByteArray(getDHDoup), VarToByteArray(getDHDuopData), resp))

		ER_CALL(parser.Parse(resp), "Errore nella lettura dei parametri di dominio DH")

		dh_g = parser.tags[0]->tags[0]->tags[0]->tags[0]->content;
		dh_p = parser.tags[0]->tags[0]->tags[0]->tags[1]->content;
		dh_q = parser.tags[0]->tags[0]->tags[0]->tags[2]->content;
	}
	else if (type == CIE_Type::CIE_NXP) {
		BYTE getDHDoup[] = { 00, 0xcb, 0x3f, 0xff };
		BYTE getDHDuopData_g[] = { 0x4D, 0x0A, 0x70, 0x08, 0xBF, 0xA1, 0x01, 0x04, 0xA3, 0x02, 0x97, 0x00 };
		CARD_R_CALL(SendAPDU(VarToByteArray(getDHDoup), VarToByteArray(getDHDuopData_g), resp))
		ER_CALL(parser.Parse(resp), "Errore nella lettura del parametri di dominio DH g")
		dh_g = parser.tags[0]->tags[0]->tags[0]->tags[0]->content;

		BYTE getDHDuopData_p[] = { 0x4D, 0x0A, 0x70, 0x08, 0xBF, 0xA1, 0x01, 0x04, 0xA3, 0x02, 0x98, 0x00 };
		CARD_R_CALL(SendAPDU(VarToByteArray(getDHDoup), VarToByteArray(getDHDuopData_p), resp))
		ER_CALL(parser.Parse(resp), "Errore nella lettura del parametri di dominio DH p")
		dh_p = parser.tags[0]->tags[0]->tags[0]->tags[0]->content;

		BYTE getDHDuopData_q[] = { 0x4D, 0x0A, 0x70, 0x08, 0xBF, 0xA1, 0x01, 0x04, 0xA3, 0x02, 0x99, 0x00 };
		CARD_R_CALL(SendAPDU(VarToByteArray(getDHDoup), VarToByteArray(getDHDuopData_q), resp))
		ER_CALL(parser.Parse(resp), "Errore nella lettura del parametri di dominio DH q")
		dh_q = parser.tags[0]->tags[0]->tags[0]->tags[0]->content;
	}
	else 
		throw CStringException("CIE non riconosciuta");


	exit_func
}

CASNTag *GetTag(CASNTagArray &tags, DWORD id) {
	for (int i = 0; i < tags.size(); i++) {
		if (tags[i]->tagInt() == id)
			return tags[i];
	}
	return nullptr;
}

void IAS::InitExtAuthKeyParam() {
	init_func
	ByteDynArray resp;
	SelectAID_CIE();

	BYTE getKeyDoup[] = { 00, 0xcb, 0x3f, 0xff };
	BYTE getKeyDuopData[] = { 0x4d, 0x0C, 0x70, 0x0A, 0xBF, 0xA0, CIE_KEY_ExtAuth_ID & 0x7f, 0x06, 0x7F, 0x49, 0x03, 0x5F, 0x20, 0x80 };
	CARD_R_CALL(SendAPDU(VarToByteArray(getKeyDoup), VarToByteArray(getKeyDuopData), resp))

	CASNParser parser;
	ER_CALL(parser.Parse(resp), "Errore nella lettura dei parametri della chiave di ExtAuth")

	CA_module = GetTag(parser.tags[0]->tags[0]->tags[0]->tags,0x81)->content;
	CA_pubexp = GetTag(parser.tags[0]->tags[0]->tags[0]->tags, 0x82)->content;	
	CA_privexp = baExtAuth_PrivExp;
	CA_CHR = GetTag(parser.tags[0]->tags[0]->tags[0]->tags, 0x5F20)->content;
	CA_CHA = GetTag(parser.tags[0]->tags[0]->tags[0]->tags, 0x5F4C)->content;
	CA_CAR = CA_CHR.mid(4);
	CA_AID = CA_CHA.left(6);
	exit_func
}

void IAS::SetCardContext(void* pCardData) {
	token.setTransmitCallbackData(pCardData);
}

void IAS::Deauthenticate() {
	init_func
	ER_CALL(token.Reset(true), "Errore nel reset")
	exit_func
}

extern BYTE encMod[];
extern BYTE encPriv[];
extern BYTE encPub[];

void IAS::InitEncKey() {
	// uso la chiave di intAuth per i servizi per decifrare il certificato
	RESULT _call_ris;

	String strPAN;
	dumpHexData(PAN.mid(5, 6), strPAN, false);

	BYTE mseSet[] = { 0x00, 0x22, 0x41, 0xa4 };
	BYTE mseSetData[] = { 0x80, 0x01, 0x02, 0x84, 0x01, 0x83 };
	ByteDynArray resp;
	if (sessSSC.isEmpty()) {
		CARD_R_CALL(SendAPDU(VarToByteArray(mseSet), VarToByteArray(mseSetData), resp))
		BYTE intAuth[] = { 0x00, 0x88, 0x00, 0x00 };
		CARD_R_CALL(SendAPDU(VarToByteArray(intAuth), strPAN.toByteArray(), resp))
	}
	else {
		CARD_R_CALL(SendAPDU_SM(VarToByteArray(mseSet), VarToByteArray(mseSetData), resp))
		BYTE intAuth[] = { 0x00, 0x88, 0x00, 0x00 };
		CARD_R_CALL(SendAPDU_SM(VarToByteArray(intAuth), strPAN.toByteArray(), resp))
	}

	CSHA256 sha256;
	ByteDynArray keySha;
	sha256.Digest(resp, keySha);
	CardEncKey = keySha.left(32);
}

void IAS::SetCache(char *PAN, ByteArray &certificate, ByteArray &FirstPIN) {
	ByteDynArray encCert, encPIN;
	CAES enc(CardEncKey);
	enc.Encode(certificate, encCert);
	enc.Encode(FirstPIN, encPIN);
	CacheSetData(PAN, encCert.lock(), encCert.size(), encPIN.lock(), encPIN.size());
}

BOOL IsUserInteractive()
{
	BOOL bIsUserInteractive = TRUE;

	HWINSTA hWinStation = GetProcessWindowStation();
	if (hWinStation != NULL)
	{
		USEROBJECTFLAGS uof = { 0 };
		if (GetUserObjectInformation(hWinStation, UOI_FLAGS, &uof, sizeof(USEROBJECTFLAGS), NULL) && ((uof.dwFlags & WSF_VISIBLE) == 0))
		{
			bIsUserInteractive = FALSE;
		}
	}
	return bIsUserInteractive;
}

void IAS::IconaSbloccoPIN() {
	if (IsUserInteractive()) {
		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		ZeroMem(si);
		si.cb = sizeof(STARTUPINFO);

		WORD getHandle = 0xfffd;
		ByteDynArray resp;
		token.Transmit(VarToByteArray(getHandle), &resp);
		SCARDHANDLE hCard = *(SCARDHANDLE*)resp.lock();
		if (!CreateProcess(nullptr, String().printf("rundll32.exe \"%s\",SbloccoPIN ICON", moduleInfo.szModuleFullPath.lock()).lock(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) 
			throw CStringException("Errore in creazione processo SbloccoPIN");
		else {
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}		
	}
}

void IAS::GetFirstPIN(ByteDynArray &PIN) {
	String PANStr;
	dumpHexData(PAN.mid(5, 6), PANStr, false);
	BYTE* EncPINBuf;
	int EncPINSize;
	CacheGetPIN(PANStr.lock(), EncPINBuf, EncPINSize);

	CAES enc(CardEncKey);
	enc.Decode(ByteArray(EncPINBuf, EncPINSize), PIN);
	CacheFree(EncPINBuf);
}

bool IAS::IsEnrolled() {
	String PANStr;
	dumpHexData(PAN.mid(5, 6), PANStr, false);
	return CacheExists(PANStr.lock());
}

void IAS::GetCertificate(ByteDynArray &certificate,bool askEnable) {
	if (!Certificate.isEmpty()) {
		certificate = Certificate;
		return;
	}

	String PANStr;
	dumpHexData(PAN.mid(5, 6), PANStr, false);
	if (!CacheExists(PANStr.lock())) {
		if (askEnable && IsUserInteractive()) {
			PROCESS_INFORMATION pi;
			STARTUPINFO si;
			ZeroMem(si);
			si.cb = sizeof(STARTUPINFO);

			WORD getHandle = 0xfffd;
			ByteDynArray resp;
			token.Transmit(VarToByteArray(getHandle), &resp);
			SCARDHANDLE hCard = *(SCARDHANDLE*)resp.lock();

			SCardEndTransaction(hCard, SCARD_UNPOWER_CARD);
			if (!CreateProcess(nullptr, String().printf("rundll32.exe \"%s\",AbilitaCIE %s", moduleInfo.szModuleFullPath.lock(), dumpHexData(PAN.mid(5, 6), String(), false).lock()).lock(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
				throw CStringException("Errore in creazione processo SbloccoPIN");
			else
				CloseHandle(pi.hThread);
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			SCardBeginTransaction(hCard);
		}
		else {
			certificate.clear();
			return;
		}
	}
	if (!CacheExists(PANStr.lock()))
		throw CStringException("Errore in abilitazione CIE");
	BYTE* certEncBuf;
	int certEncSize;
	CacheGetCertificate(PANStr.lock(), certEncBuf, certEncSize);

	CAES enc(CardEncKey);	
	enc.Decode(ByteArray(certEncBuf, certEncSize), certificate);
	CacheFree(certEncBuf);
	Certificate = certificate;
}

void IAS::VerificaSOD(ByteArray &SOD, std::map<BYTE, ByteDynArray> &hashSet) {
	init_func
	CASNParser parser;
	parser.Parse(SOD);

	String dump;
	dumpHexData(SOD, dump);

	CASNTag &SODTag = *parser.tags[0];

	CASNTag &temp = SODTag.Child(0, 0x30);
	byte OID[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02 };
	temp.Child(0, 06).Verify(VarToByteArray(OID));
	BYTE val3=3;
	CASNTag &temp2 = temp.Child(1, 0xA0).Child(0, 0x30);
	temp2.Child(0, 2).Verify(VarToByteArray(val3));

	byte OID_SH256[] = { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01 };
	temp2.Child(1, 0x31).Child(0, 0x30).Child(0, 6).Verify(VarToByteArray(OID_SH256));

	byte OID3[] = { 0x67, 0x81, 0x08, 0x01, 0x01, 0x01 };
	temp2.Child(2, 0x30).Child(0, 06).Verify(VarToByteArray(OID3));
	ByteArray ttData = temp2.Child(2, 0x30).Child(1, 0xA0).Child(0, 04).content;
	CASNParser ttParser;
	ttParser.Parse(ttData);
	CASNTag &signedData = *ttParser.tags[0];
	signedData.CheckTag(0x30);

	CASNTag &signerCert = temp2.Child(3, 0xA0).Child(0, 0x30);
	CASNTag &temp3 = temp2.Child(4, 0x31).Child(0, 0x30);
	BYTE val1 = 1;
	temp3.Child(0, 02).Verify(VarToByteArray(val1));
	CASNTag &issuerName = temp3.Child(1, 0x30).Child(0, 0x30);
	CASNTag &signerCertSerialNumber = temp3.Child(1, 0x30).Child(1, 02);
	temp3.Child(2, 0x30).Child(0, 06).Verify(VarToByteArray(OID_SH256));

	CASNTag &signerInfo = temp3.Child(3, 0xA0);
	byte OID4[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x03 };
	signerInfo.Child(0, 0x30).Child(0, 06).Verify(VarToByteArray(OID4));
	byte OID5[] = { 0x67, 0x81, 0x08, 0x01, 0x01, 0x01 };
	signerInfo.Child(0, 0x30).Child(1, 0x31).Child(0, 06).Verify(VarToByteArray(OID5));
	byte OID6[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x04 };
	signerInfo.Child(1, 0x30).Child(0, 06).Verify(VarToByteArray(OID6));
	CASNTag &digest = temp3.Child(3, 0xA0).Child(1, 0x30).Child(1, 0x31).Child(0, 04);

	byte OID_RSAwithSHA256[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0b };
	byte OID_RSAwithSHA1[] = { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05 };
	auto &digestAlgo = temp3.Child(4, 0x30).Child(0, 06).content;
	bool isSHA1 = false;
	bool isSHA256 = false;
	if (digestAlgo == VarToByteArray(OID_RSAwithSHA1))
		isSHA1 = true;
	else 
		if (digestAlgo == VarToByteArray(OID_RSAwithSHA256))
			isSHA256 = true;
		else
			throw CStringException("Algoritmo del digest della firma non valido");

	CASNTag &signature = temp3.Child(5, 04);
	
	CSHA256 sha256;
	ByteDynArray calcDigest;
	sha256.Digest(ttData.mid((int)signedData.startPos, (int)(signedData.endPos - signedData.startPos)), calcDigest);
	if (calcDigest!=digest.content)
		throw CStringException("Digest del SOD non corrispondente ai dati");

	ByteArray certRaw = SOD.mid((int)signerCert.startPos, (int)(signerCert.endPos - signerCert.startPos));
	PCCERT_CONTEXT certDS =CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certRaw.lock(), certRaw.size());

	ByteArray pubKeyData(certDS->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData, certDS->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData);
	CASNParser pubKeyParser;
	pubKeyParser.Parse(pubKeyData);
	CASNTag &pubKey = *pubKeyParser.tags[0];
	CASNTag &modTag = pubKey.Child(0, 02);
	ByteArray mod = modTag.content;
	while (mod[0] == 0)
		mod = mod.mid(1);
	CASNTag &expTag = pubKey.Child(1, 02);
	ByteArray exp = expTag.content;
	while (exp[0] == 0)
		exp = exp.mid(1);

	ByteArray signatureData = signature.content;

	CRSA rsa(mod, exp);

	ByteDynArray decryptedSignature;
	rsa.RSA_PURE(signatureData, decryptedSignature);
	decryptedSignature = decryptedSignature.mid(RemovePaddingBT1(decryptedSignature));
	ByteArray toSign = SOD.mid((int)signerInfo.tags[0]->startPos, (int)(signerInfo.tags[signerInfo.tags.size()- 1]->endPos - signerInfo.tags[0]->startPos));
	ByteDynArray digestSignature;
	if (isSHA1) {
		CSHA1 sha1;
		decryptedSignature = decryptedSignature.mid(RemoveSha1(decryptedSignature));
		sha1.Digest(toSign.setASN1Tag(0x31, ByteDynArray()), digestSignature);
	}
	if (isSHA256) {
		CSHA256 sha256;
		decryptedSignature = decryptedSignature.mid(RemoveSha256(decryptedSignature));
		sha256.Digest(toSign.setASN1Tag(0x31, ByteDynArray()), digestSignature);
	}
	if (digestSignature!=decryptedSignature)
		throw CStringException("Firma del SOD non valida");

	//log.Info("Verifica issuer");

	issuerName.Reparse();
	CASNParser issuerParser;
	issuerParser.Parse(ByteArray(certDS->pCertInfo->Issuer.pbData, certDS->pCertInfo->Issuer.cbData));
	CASNTag &CertIssuer = *issuerParser.tags[0];
	if (issuerName.tags.size() != CertIssuer.tags.size())
		throw CStringException("Issuer name non corrispondente");
	for (int i = 0; i < issuerName.tags.size(); i++) {
		CASNTag &certElem = *CertIssuer.tags[i]->tags[0];
		CASNTag &SODElem = *issuerName.tags[i]->tags[0];
		certElem.tags[0]->Verify(SODElem.tags[0]->content);
		certElem.tags[1]->Verify(SODElem.tags[1]->content);
	}

	ByteDynArray certSerial(certDS->pCertInfo->SerialNumber.pbData, certDS->pCertInfo->SerialNumber.cbData);
	if (certSerial.reverse() != signerCertSerialNumber.content)
		throw CStringException("Serial Number del certificato non corrispondente");

	// ora verifico gli hash dei DG
	//log.Info("Verifica hash DG");
	BYTE val0=0;
	signedData.Child(0, 02).Verify(VarToByteArray(val0));
	signedData.Child(1, 0x30).Child(0, 06).Verify(VarToByteArray(OID_SH256));
	
	CASNTag &hashTag = signedData.Child(2, 0x30);
	for (int i = 0; i<hashTag.tags.size();i++) {
		CASNTag &hashDG = *(hashTag.tags[i]);
		CASNTag &dgNum = hashDG.CheckTag(0x30).Child(0, 02);
		CASNTag &dgHash = hashDG.Child(1, 04);
		BYTE num = ByteArrayToVar(dgNum.content, BYTE);

		if (hashSet.find(num) == hashSet.end() || hashSet[num].size() == 0)
			throw CStringException(String().printf("Digest non trovato per il DG %02X", num).lock());
		
		if (hashSet[num] != dgHash.content)
			throw CStringException(String().printf("Digest non corrispondente per il DG %02X", num).lock());
	}

	/*if (CSCA != null && CSCA.Count > 0)
	{
		log.Info("Verifica catena CSCA");
		X509CertChain chain = new X509CertChain(CSCA);
		var certChain = chain.getPath(certDS);
		if (certChain == null)
			throw Exception("Il certificato di Document Signer non � valido");

		var rootCert = certChain[0];
		if (!new ByteArray(rootCert.SubjectName.RawData).IsEqual(rootCert.IssuerName.RawData))
			throw Exception("Impossibile validare il certificato di Document Signer");
	}
	*/
	exit_func
}

#define DWL_MSGRESULT 0

BOOL CheckOneInstance(char *nome)
{
	auto m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, nome);
	if (GetLastError() == ERROR_ALREADY_EXISTS && m_hStartEvent != nullptr) {
		CloseHandle(m_hStartEvent);
		m_hStartEvent = nullptr;
		return FALSE;
	}
	return TRUE;
}

