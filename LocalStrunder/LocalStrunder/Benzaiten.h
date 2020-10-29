#define CONST_UnrRotToRad                          0.00009587379924285
#define CONST_Pi                                   3.1415926535897932
char KeyGenerate[] = { '?','1','=','?','+','.','.','3','8','2',0 };
unsigned char rc4i;
unsigned char rc4j;
char rc4arr[256];

template< class T > struct TArray
{
public:
	T* Data;
	int Count;
	int Max;

public:
	TArray()
	{
		Data = NULL;
		Count = Max = 0;
	};

public:
	int Num()
	{
		return this->Count;
	};

	T& operator() (int i)
	{
		return this->Data[i];
	};

	const T& operator() (int i) const
	{
		return this->Data[i];
	};

	void Add(T InputData)
	{
		Data = (T*)realloc(Data, sizeof(T) * (Count + 1));
		Data[Count++] = InputData;
		Max = Count;
	};

	void Clear()
	{
		free(Data);
		Count = Max = 0;
	};
};

struct FRotator
{
	int                                                Pitch;                                            		// 0x0000 (0x0004) [0x0000000000000001]              ( CPF_Edit )
	int                                                Yaw;                                              		// 0x0004 (0x0004) [0x0000000000000001]              ( CPF_Edit )
	int                                                Roll;                                             		// 0x0008 (0x0004) [0x0000000000000001]              ( CPF_Edit )
};

struct Vector3
{
	float                                              X;                                                		// 0x0000 (0x0004) [0x0000000000000001]              ( CPF_Edit )
	float                                              Y;                                                		// 0x0004 (0x0004) [0x0000000000000001]              ( CPF_Edit )
	float                                              Z;                                                		// 0x0008 (0x0004) [0x0000000000000001]              ( CPF_Edit )
	Vector3()
	{

	}
	Vector3(float x, float y, float z)
	{
		this->X = x;
		this->Y = y;
		this->Z = z;
	}
	Vector3 operator/(const Vector3& V) const
	{
		return Vector3(X / V.X, Y / V.Y, Z / V.Z);
	}
};

struct FString : public TArray< wchar_t >
{
	FString() {};

	FString(wchar_t* Other)
	{
		this->Max = this->Count = *Other ? (wcslen(Other) + 1) : 0;

		if (this->Count)
			this->Data = Other;
	};

	~FString() {};

	FString operator = (wchar_t* Other)
	{
		if (this->Data != Other)
		{
			this->Max = this->Count = *Other ? (wcslen(Other) + 1) : 0;

			if (this->Count)
				this->Data = Other;
		}

		return *this;
	};
};

class PlayerData {
public:
	DWORD PlayerController;
	DWORD Pointer;
	Vector3 W2S;
	Vector3 Location3D;
	Vector3 CameraLocation;
	FRotator CameraRotation;
	int Health;
	int HealthMax;
	int TeamIndex;
	float FOVAngle;
	float EyeHeight;
	float ExactPing;
	FString PlayerName;
	inline float DistTo(const Vector3& vOther) const;
};

float PlayerData::DistTo(const Vector3& vOther) const
{
	return sqrt(pow(vOther.X - Location3D.X, 2)
		+ pow(vOther.Y - Location3D.Y, 2)
		+ pow(vOther.Z - Location3D.Z, 2));
}

float Size(Vector3& v)
{
	return sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
}

float inline Dot(const Vector3& V1, const Vector3& V2)
{
	return (V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z);
}

void Normalize(Vector3& v)
{
	float size = Size(v);
	if (!size)
	{
		v.Y = v.Y = v.Z = 1;
	}
	else
	{
		v.X /= size;
		v.Y /= size;
		v.Z /= size;
	}
}

Vector3 RotationToVector(FRotator R)
{
	Vector3 Vec;
	float fYaw = R.Yaw * CONST_UnrRotToRad;
	float fPitch = R.Pitch * CONST_UnrRotToRad;
	float CosPitch = cos(fPitch);
	Vec.X = cos(fYaw) * CosPitch;
	Vec.Y = sin(fYaw) * CosPitch;
	Vec.Z = sin(fPitch);
	return Vec;
}

void GetAxes(FRotator R, Vector3& X, Vector3& Y, Vector3& Z)
{
	X = RotationToVector(R);
	Normalize(X);
	R.Yaw += 16384;
	FRotator R2 = R;
	R2.Pitch = 0.f;
	Y = RotationToVector(R2);
	Normalize(Y);
	Y.Z = 0.f;
	R.Yaw -= 16384;
	R.Pitch += 16384;
	Z = RotationToVector(R);
	Normalize(Z);
}

float VectorDotProduct(Vector3& A, Vector3& B)
{
	return ((A.X * B.X) + (A.Y * B.Y) + (A.Z * B.Z));
}

Vector3 WorldToScreen(Vector3 Location, float FOV, PlayerData myData, PlayerData curPawn, int Width, int Height)
{
	Vector3 Return;
	Vector3 AxisX, AxisY, AxisZ, Delta, Transformed;
	GetAxes(myData.CameraRotation, AxisX, AxisY, AxisZ);
	Transformed.X = VectorDotProduct(Delta, AxisY);
	Transformed.Y = VectorDotProduct(Delta, AxisZ);
	Transformed.Z = VectorDotProduct(Delta, AxisX);
	Delta.X = Location.X - myData.CameraLocation.X;
	Delta.Y = Location.Y - myData.CameraLocation.Y;
	Delta.Z = Location.Z - myData.CameraLocation.Z;
	Transformed.X = (Delta.X * AxisY.X) + (Delta.Y * AxisY.Y) + (Delta.Z * AxisY.Z);
	Transformed.Y = (Delta.X * AxisZ.X) + (Delta.Y * AxisZ.Y) + (Delta.Z * AxisZ.Z);
	Transformed.Z = (Delta.X * AxisX.X) + (Delta.Y * AxisX.Y) + (Delta.Z * AxisX.Z);
	if (Transformed.Z < 1)
		Transformed.Z = 1;
	Return.X = (Width / 2.0f) + Transformed.X * ((Width / 2.0f) / tan(FOV * CONST_Pi / 360.0f)) / Transformed.Z;
	Return.Y = (Height / 2.0f) + -Transformed.Y * ((Width / 2.0f) / tan(FOV * CONST_Pi / 360.0f)) / Transformed.Z;
	return Return;
}

//Security Lock

unsigned long GetSRNumber() {
	unsigned int drive_type = GetDriveType(NULL);
	char VolumeNameBuffer[100];
	char FileSystemNameBuffer[100];
	LPDWORD fs = 0, sz = 0;
	unsigned long drive_sn;
	GetVolumeInformationA(
		NULL,
		VolumeNameBuffer,
		100,
		&drive_sn,
		sz,
		fs,
		FileSystemNameBuffer,
		100
		);
	return drive_sn;
}

char* MD5(char* data, int size)
{
	HCRYPTHASH hHash;
	HCRYPTPROV hProv;
	BYTE md5hash[16];
	DWORD md5hash_size, dwSize;
	static char str_hash[33];
	int i;

	ZeroMemory(str_hash, sizeof(str_hash));
	ZeroMemory(md5hash, sizeof(md5hash));
	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0);
	CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	CryptHashData(hHash, (BYTE*)data, size, 0);
	dwSize = sizeof(md5hash_size);
	CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&md5hash_size, &dwSize, 0);
	CryptGetHashParam(hHash, HP_HASHVAL, md5hash, &md5hash_size, 0);
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	for (i = 0; i < md5hash_size; i++)
		sprintf(str_hash + 2 * i, "%2.2X", md5hash[i]);
	return str_hash;
}

void rc4init(char* k)
{
	char t;
	char tmp[256];

	rc4j = strlen(k); rc4i = '\x00';
	do { rc4arr[rc4i] = rc4i; tmp[rc4i] = k[rc4i % rc4j]; } while (++rc4i);
	rc4i = rc4j = '\x00';
	do {
		rc4j += rc4arr[rc4i] + tmp[rc4i];
		t = rc4arr[rc4i];
		rc4arr[rc4i] = rc4arr[rc4j];
		rc4arr[rc4j] = t;
	} while (++rc4i);
}

void rc4(char* str, int len)
{
	char t;
	while (len--) {
		rc4j += rc4arr[++rc4i];
		t = rc4arr[rc4i];
		rc4arr[rc4i] = rc4arr[rc4j];
		rc4arr[rc4j] = t;
		*(str++) ^= rc4arr[(char)(rc4arr[rc4i] + rc4arr[rc4j])];
	}
}

char* GenerateKey(char* Name)
{
	CHAR BUFFER_KEY[255] = { 0 };
	ZeroMemory(&BUFFER_KEY, 255);

	sprintf(BUFFER_KEY, "%s%d", Name, GetSRNumber());

	int len = strlen(BUFFER_KEY);
	rc4init(KeyGenerate);
	rc4(BUFFER_KEY, len);

	return MD5(BUFFER_KEY, strlen(BUFFER_KEY));
}