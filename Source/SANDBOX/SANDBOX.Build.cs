using UnrealBuildTool;

public class SANDBOX : ModuleRules
{
    public SANDBOX(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Добавляем модули для работы с геометрией и физикой Chaos
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "GeometryCollectionEngine", // <--- ОБЯЗАТЕЛЬНО ДЛЯ UGeometryCollectionComponent
			"ChaosSolverEngine",        // <--- Нужно для физики разрушения
            "Niagara",
            "PhysicsCore"               // <--- Базовая физика
		});

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");
    }
}