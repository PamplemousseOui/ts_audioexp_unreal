#pragma once

#include "AkAudioDevice.h"
#include "AkAudioType.h"
#include "Serialization/BulkData.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/Object.h"

#include "AkAssetBase.generated.h"

class ISoundBankInfoCache;
class UAkMediaAsset;

struct FStreamableHandle;

UCLASS()
class AKAUDIO_API UAkAssetData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AkAssetBase")
	uint32 CachedHash;

	FByteBulkData Data;

public:
	void Serialize(FArchive& Ar) override;

	virtual AKRESULT Load();
	virtual AKRESULT Unload();

#if WITH_EDITOR
	virtual void GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const;
#endif

	virtual bool IsMediaReady() const;

	FAkAudioDevice::SetCurrentAudioCultureAsyncTask* parentSetCultureTask = nullptr;
	friend class AkEventBasedIntegrationBehavior;

protected:
	AkBankID BankID = AK_INVALID_BANK_ID;
	const void* RawData = nullptr;

#if WITH_EDITOR
	TArray<uint8> EditorRawData;
#endif
};

UCLASS() 
class AKAUDIO_API UAkAssetDataWithMedia : public UAkAssetData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "AkAssetBase")
	TArray<TSoftObjectPtr<UAkMediaAsset>> MediaList;

	AKRESULT Load() override;
	AKRESULT Unload() override;

	bool IsMediaReady() const override;

#if WITH_EDITOR
	void GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const override;
#endif

protected:
	TSharedPtr<FStreamableHandle> mediaStreamHandle;
};

UCLASS()
class AKAUDIO_API UAkAssetPlatformData : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "AkAssetBase")
	TMap<FString, UAkAssetData*> AssetDataPerPlatform;
#endif

	UPROPERTY()
	UAkAssetData* CurrentAssetData;

public:
	void Serialize(FArchive& Ar) override;

#if WITH_EDITOR
	void Reset();

	bool NeedsRebuild(const TSet<FString>& PlatformsToBuild, const FString& Language, const FGuid& ID, const ISoundBankInfoCache* SoundBankInfoCache) const;

	void GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const;
#endif
};

UCLASS()
class AKAUDIO_API UAkAssetBase : public UAkAudioType
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "AkAssetBase")
	UAkAssetPlatformData* PlatformAssetData = nullptr;

public:
	void FinishDestroy() override;

	virtual void Load();
	virtual void Unload();

#if WITH_EDITOR
	template<typename T>
	T* FindOrAddAssetDataTyped(const FString& Platform, const FString& Language)
	{
		return Cast<T>(FindOrAddAssetData(Platform, Language));
	}

	virtual UAkAssetData* FindOrAddAssetData(const FString& Platform, const FString& Language);
	virtual void GetMediaList(TArray<TSoftObjectPtr<UAkMediaAsset>>& MediaList) const;

	virtual bool NeedsRebuild(const TSet<FString>& PlatformsToBuild, const TSet<FString>& LanguagesToBuild, const ISoundBankInfoCache* SoundBankInfoCache) const;

	void Reset() override;

	FCriticalSection BulkDataWriteLock;
#endif

protected:
	virtual UAkAssetData* createAssetData(UObject* Parent) const;
	virtual UAkAssetData* getAssetData() const;
	FAkAudioDevice::SetCurrentAudioCultureAsyncTask* parentSetCultureTask = nullptr;
	bool bDelayLoadAssetMedia = false;

	UAkAssetData* internalFindOrAddAssetData(UAkAssetPlatformData* Data, const FString& Platform, UObject* Parent);

private:
#if WITH_EDITOR
	FCriticalSection assetDataLock;
#endif
};

using SwitchLanguageCompletedFunction = TFunction<void(bool)>;