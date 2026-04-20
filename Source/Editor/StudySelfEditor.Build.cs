using UnrealBuildTool;

public class StudySelfEditor : ModuleRules
{
	public StudySelfEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 여기에는 에디터에서만 쓸 모듈을 마음껏 넣으세요. 런타임 빌드와 충돌 안 납니다.
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", 
			"StudySelf",      // 중요: 기존 메인 모듈을 참조해야 함
			"UnrealEd",       // 이제 마음껏 쓰세요
			"AssetTools",     // 이제 마음껏 쓰세요
			"HTTP", "Json" 
		});
	}
}