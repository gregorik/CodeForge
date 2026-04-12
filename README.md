[Watch it in action](https://www.youtube.com/watch?v=PeYbQVEmiRo)

[Read the manual](https://gregorigin.com/CodeForge/)

[Read the docs of the RPG Kit Example Project](https://gregorigin.com/CodeForge_RPG_Starter_Kit/)

[Get the more advanced Pro version.](https://www.fab.com/listings/14bcc805-1ed2-42f3-8d52-406b9f97b8e9)

[Discord support](https://discord.gg/nqYQ5mtmHb) <br><br>

![CodeForge0](https://github.com/user-attachments/assets/a027368a-77c5-49c9-9c98-344d2fd5d99e)

----

CodeForge (Core) is an experimental, node-based visual programming toolkit for Unreal Engine 5.6+.
Its core features are the following:
 
 * Visual Data Modeling: Provides a native UEdGraph UI where users can visually define Classes (UCLASS), Structs
     (USTRUCT), Enums (UENUM), Interfaces (UINTERFACE), and Delegates without touching an IDE.
 * Zero-Dependency Code Generation: It features a bespoke, ~500 LOC string-based template engine
     (FCodeForgeTemplateEngine) that parses .cft files. It translates the visual schema into completely UBT-compliant
     .h and .cpp files.
 * Robust Boilerplate Handling: It excels at generating boilerplate that is tedious to write manually. It
     automatically handles UPROPERTY / UFUNCTION specifiers (via checkbox UI), and fully scaffolds
     Multiplayer/Replication logic (auto-generating GetLifetimeReplicatedProps, DOREPLIFETIME, _Implementation stubs
     for RPCs, and OnRep_ callbacks).
 * Logic Injection: The recent RPG Starter Kit updates prove it can now accept raw C++ logic (FunctionBody,
     ConstructorBody) directly from the UI, generating playable source code rather than just empty headers.
 * Validation & UX: Real-time graph validation flags errors (e.g., duplicate names, invalid specifier combos)
     natively on the nodes. It supports drag-and-drop from the Content Browser, and seamlessly triggers Unreal's Live
     Coding or prompts for editor restarts depending on whether a change is "structural" or "behavioral."

 * No External Dependencies: By writing a custom C++ template engine rather than relying on an external library or
     language (like Python), CodeForge is completely self-contained. It is 100% offline and deterministic.
 * Native Integration: By making UCodeForgeBlueprint a standard UAsset and utilizing the standard UEdGraph, the
     plugin inherently inherits Epic's native undo/redo operations, Details Panel reflection, saving mechanisms, and
     source control compatibility.
 * Test-Driven: The CodeForgeRuntime and CodeForgeEditor modules are backed by a robust suite of UE5 Automation Tests
     covering template substitutions, conditionals, module scanning, and validation rules.
 * Separation of Concerns: The 4-layer architecture (Graph UI → Schema Model → Template Engine → Template Text)
     ensures that when Epic updates Unreal Engine macros in the future, the developer only needs to edit plaintext .cft
     files, without recompiling the core plugin.

 * The primary market is intermediate Unreal developers who understand Blueprints but are intimidated by C++ syntax,
     header/source splitting, and #include rules. CodeForge acts as training wheels,
     outputting best-practice C++ they can study and extend.
 * The Power User Pitch (Scaffolding): For senior developers, writing DECLARE_DYNAMIC_MULTICAST_DELEGATE or setting
     up GetLifetimeReplicatedProps takes time. CodeForge reduces a 10-minute IDE chore to a 10-second node connection.
 * Vs. AI Competitors: Other tools on the market (like BP2AI or Node Code Editor) rely on ChatGPT/Claude APIs. They
     require internet connections, cost API credits, and suffer from LLM hallucinations. CodeForge's "Deterministic,
     Offline, Zero-Cost" angle is a massive USP.
 * The Playable Showcase: The inclusion of the "RPG Starter Kit" Example (which generates a playable Character with
     health, damage, and inventory using console commands) gives the product immediate street cred. This isn't just
     a toy: it outputs real, compiling gameplay logic out of the box.

More info to come.
