// Fill out your copyright notice in the Description page of Project Settings.


#include "DES.h"

void DES::GenerateKeys()
{
	uint8 pc1[56] = {
		57,49,41,33,25,17,9,
		1,58,50,42,34,26,18,
		10,2,59,51,43,35,27,
		19,11,3,60,52,44,36,
		63,55,47,39,31,23,15,
		7,62,54,46,38,30,22,
		14,6,61,53,45,37,29,
		21,13,5,28,20,12,4
	};

	uint8 pc2[48] = {
		14,17,11,24,1,5,
		3,28,15,6,21,10,
		23,19,12,4,26,8,
		16,7,27,20,13,2,
		41,52,31,37,47,55,
		30,40,51,45,33,48,
		44,49,39,56,34,53,
		46,42,50,36,29,32
	};

	TBitArray<> keyBitArray = TArray8ToTBitArray(key);
	TBitArray<> permKeyBitArray(0, 56);

	for (int i = 0; i < 56; i++) {
		permKeyBitArray[i] = keyBitArray[pc1[i] - 1];
	}

	TBitArray<> leftBitArray(0, 28);
	TBitArray<> rightBitArray(0, 28);

	leftBitArray.SetRangeFromRange(0, 28, permKeyBitArray, 0);
	rightBitArray.SetRangeFromRange(0, 28, permKeyBitArray, 28);

	for (int i = 0; i < 16; i++) {
		if (i == 0 || i == 1 || i == 8 || i == 15) {
			leftBitArray.Add(leftBitArray[0]);
			rightBitArray.Add(rightBitArray[0]);
			leftBitArray.RemoveAt(0, 1);
			rightBitArray.RemoveAt(0, 1);
		}
		else {
			leftBitArray.Add(leftBitArray[0]);
			leftBitArray.Add(leftBitArray[1]);
			rightBitArray.Add(rightBitArray[0]);
			rightBitArray.Add(rightBitArray[1]);
			leftBitArray.RemoveAt(0, 2);
			rightBitArray.RemoveAt(0, 2);
		}

		TBitArray<> combinedKeyBitArray;
		combinedKeyBitArray.AddRange(leftBitArray, 28, 0);
		combinedKeyBitArray.AddRange(rightBitArray, 28, 0);

		TBitArray<> roundKeyBitArray(0, 48);

		for (int j = 0; j < 48; j++) {
			roundKeyBitArray[j] = combinedKeyBitArray[pc2[j] - 1];
		}

		keys.Add(roundKeyBitArray);
	}
}

TBitArray<> DES::TArray8ToTBitArray(TArray<uint8> tArray8)
{
	int length8 = tArray8.Num();
	int length32 = length8 / 4;

	TArray<uint32> tArray32;
	for (int i = 0; i < length32; i++) {
		uint32 temp = 0;

		for (int j = 0; j < 4; j++) {
			uint8 u8 = (uint32)tArray8[i * 4 + j];
			for (int k = 0; k < 8; k++) {
				temp = temp | (u8 >> (7 - k) & 1) << (j * 8 + k);
			}
		}
		tArray32.Add(temp);
	}
	
	TBitArray<> bitArray;
	bitArray.AddRange(tArray32.GetData(), length32 * 32, 0);
	return bitArray;
}

DES::DES()
{
	key.Init(0, 8);
	GenerateKeys();
}

DES::DES(TArray<uint8> key)
{
	this->key = key;
	GenerateKeys();
}

DES::~DES()
{
}

TArray<uint8> DES::Encrypt(TArray<uint8> data)
{
	TBitArray<> pt = TArray8ToTBitArray(data);

	uint8 initialPermutation[64] = {
		58,50,42,34,26,18,10,2,
		60,52,44,36,28,20,12,4,
		62,54,46,38,30,22,14,6,
		64,56,48,40,32,24,16,8,
		57,49,41,33,25,17,9,1,
		59,51,43,35,27,19,11,3,
		61,53,45,37,29,21,13,5,
		63,55,47,39,31,23,15,7
	};

	uint8 expansionTable[48] = {
		32,1,2,3,4,5,4,5,
		6,7,8,9,8,9,10,11,
		12,13,12,13,14,15,16,17,
		16,17,18,19,20,21,20,21,
		22,23,24,25,24,25,26,27,
		28,29,28,29,30,31,32,1
	};

	int substitionBoxes[8][4][16] =
	{ {
		14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
		0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
		4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
		15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
	},
	{
		15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
		3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
		0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
		13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
	},
	{
		10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
		13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
		13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
		1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
	},
	{
		7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
		13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
		10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
		3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
	},
	{
		2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
		14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
		4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
		11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
	},
	{
		12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
		10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
		9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
		4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
	},
	{
		4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
		13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
		1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
		6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
	},
	{
		13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
		1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
		7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
		2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
	} };

	uint8 permutationTab[32] = {
		16,7,20,21,29,12,28,17,
		1,15,23,26,5,18,31,10,
		2,8,24,14,32,27,3,9,
		19,13,30,6,22,11,4,25
	};

	uint8 inversePermutation[64] = {
		40,8,48,16,56,24,64,32,
		39,7,47,15,55,23,63,31,
		38,6,46,14,54,22,62,30,
		37,5,45,13,53,21,61,29,
		36,4,44,12,52,20,60,28,
		35,3,43,11,51,19,59,27,
		34,2,42,10,50,18,58,26,
		33,1,41,9,49,17,57,25
	};

	TBitArray<> permBitArray(0, 64);
	for (int i = 0; i < 64; i++) {
		permBitArray[i] = pt[initialPermutation[i] - 1];
	}

	TBitArray<> leftBitArray(0, 32);
	TBitArray<> rightBitArray(0, 32);
	leftBitArray.SetRangeFromRange(0, 32, permBitArray, 0);
	rightBitArray.SetRangeFromRange(0, 32, permBitArray, 32);
 
	for (int i = 0; i < 16; i++) {
		TBitArray<> rightExpanded(0, 48);

		for (int j = 0; j < 48; j++) {
			rightExpanded[j] = rightBitArray[expansionTable[j] - 1];
		}
		TBitArray<> xored = TBitArray<>::BitwiseXOR(keys[i], rightExpanded, EBitwiseOperatorFlags::MaxSize);
		TBitArray<> res;
		TArray<uint8> res8;

		for (int j = 0; j < 8; j++) {
			uint8 row = (uint8)xored[j * 6] << 1 | (uint8)xored[j * 6 + 5];
			uint8 col = (uint8)xored[j * 6 + 1] << 3 | (uint8)xored[j * 6 + 2] << 2 | (uint8)xored[j * 6 + 3] << 1 | (uint8)xored[j * 6 + 4];

			uint8 val = substitionBoxes[j][row][col];
			res.Add(val >> 3 & 1);
			res.Add(val >> 2 & 1);
			res.Add(val >> 1 & 1);
			res.Add(val >> 0 & 1);
		}

		TBitArray<> perm2(0, 32);
		for (int j = 0; j < 32; j++) {
			perm2[j] = res[permutationTab[j] - 1];
		}

		xored = TBitArray<>::BitwiseXOR(perm2, leftBitArray, EBitwiseOperatorFlags::MaxSize);

		leftBitArray = xored;
		if (i < 15) {
			TBitArray<> temp = rightBitArray;
			rightBitArray = xored;
			leftBitArray = temp;
		}
	}

	TBitArray<> combinedBitArray;
	combinedBitArray.AddRange(leftBitArray, 32);
	combinedBitArray.AddRange(rightBitArray, 32);

	TBitArray<> cipherBitArray(0, 64);

	for (int i = 0; i < 64; i++) {
		cipherBitArray[i] = combinedBitArray[inversePermutation[i] - 1];
	}
	TArray<uint8> cipherArray; 
	
	for (int i = 0; i < 8; i++) {
		uint8 temp = 0;

		for (int j = 0; j < 8; j++) {
			temp = temp << 1 | (uint8)cipherBitArray[i * 8 + j];
		}

		cipherArray.Add(temp);
	}
	
	return cipherArray;
}