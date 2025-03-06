// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dom/JsonValue.h"
#include "DTBPJsonBPLib.generated.h"


USTRUCT(BlueprintInternalUseOnly)
struct FDTStruct {
	GENERATED_USTRUCT_BODY()
};

/**
 * 
 */
UCLASS()
class DTBPJSON_API UDTBPJsonBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, CustomThunk, Category = "DT Json", meta = (DisplayName = "Struct To Json", CustomStructureParam = "Struct", AdvancedDisplay = "PrettyPrint"))
	static void StructToJson(const FDTStruct& Struct, FString& Json, bool PrettyPrint = false);
	static void Generic_StructToJson(const void* StructAddr, const FStructProperty* StructProperty, FString& Json, bool PrettyPrint);

	UFUNCTION(BlueprintPure, CustomThunk, Category = "DT Json", meta = (DisplayName = "Json To Struct", CustomStructureParam = "Struct"))
	static void JsonToStruct(FDTStruct& Struct, const FString& Json);
	static void Generic_JsonToStruct(void* StructAddr, FStructProperty* StructProperty, const FString& Json);

public:
	DECLARE_FUNCTION(execStructToJson)
	{
		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* StructAddr = Stack.MostRecentPropertyAddress;
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		P_GET_PROPERTY_REF(FStrProperty, Json);
		P_GET_PROPERTY(FBoolProperty, PrettyPrint);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_StructToJson(StructAddr, StructProperty, Json, PrettyPrint);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execJsonToStruct)
	{
		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* StructAddr = Stack.MostRecentPropertyAddress;
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		P_GET_PROPERTY(FStrProperty, Json);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_JsonToStruct(StructAddr, StructProperty, Json);
		P_NATIVE_END;
	}

public:
	static bool UStructToJsonObjectString(const UStruct* StructDefinition, const void* Struct, FString& OutJsonString, int64 CheckFlags, int64 SkipFlags, int32 Indent, bool bPrettyPrint);
	static bool UStructToJsonObject(const UStruct* StructDefinition, const void* Struct, TSharedRef<FJsonObject> OutJsonObject, int64 CheckFlags, int64 SkipFlags);
	static bool UStructToJsonAttributes(const UStruct* StructDefinition, const void* Struct, TMap< FString, TSharedPtr<FJsonValue> >& OutJsonAttributes, int64 CheckFlags, int64 SkipFlags);
	static TSharedPtr<FJsonValue> UPropertyToJsonValue(FProperty* Property, const void* Value, int64 CheckFlags, int64 SkipFlags, FProperty* OuterProperty);
	static TSharedPtr<FJsonValue> ConvertScalarFPropertyToJsonValue(FProperty* Property, const void* Value, int64 CheckFlags, int64 SkipFlags, FProperty* OuterProperty);

	static bool JsonAttributesToUStructWithContainer(const TMap< FString, TSharedPtr<FJsonValue> >& JsonAttributes, const UStruct* StructDefinition, void* OutStruct, const UStruct* ContainerStruct, void* Container, int64 CheckFlags, int64 SkipFlags);
	static bool JsonValueToFPropertyWithContainer(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue, const UStruct* ContainerStruct, void* Container, int64 CheckFlags, int64 SkipFlags);
	static bool ConvertScalarJsonValueToFPropertyWithContainer(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue, const UStruct* ContainerStruct, void* Container, int64 CheckFlags, int64 SkipFlags);
};
