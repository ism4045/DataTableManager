#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataTableManagerConfig.generated.h"

/**
 * 
 */
UCLASS(Config = Editor)
class DATATABLEMODULE_API UDataTableManagerConfig : public UObject
{
	GENERATED_BODY()

public:
	UDataTableManagerConfig();

	UPROPERTY(Config, EditAnywhere, Category = "Paths")
	FString CachedExcelPath;

	UPROPERTY(Config, EditAnywhere, Category = "Paths")
	FString CachedCSVPath;

	UPROPERTY(Config, EditAnywhere, Category = "Paths")
	FString CachedStructPath;

	UPROPERTY(Config, EditAnywhere, Category = "Paths")
	FString CachedAssetPath;
};
