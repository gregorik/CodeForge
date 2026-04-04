# CodeForge Core

![CodeForge Banner](https://raw.githubusercontent.com/gregorik/CodeForge/main/Resources/Icon128.png)

**CodeForge Core** is an open-source, node-based C++ scaffolding tool for Unreal Engine 5.6+. It allows developers to visually define data models�Classes, Structs, and Enums�directly within the Unreal Editor, automatically generating UBT-compliant, best-practice C++ code.

> [!TIP]
> **Escape "Tutorial Hell":** CodeForge Core is designed for developers who understand Blueprints but find C++ syntax, header/source splitting, and `#include` rules intimidating.

---

## ?? Key Features

### 1. Visual Data Modeling
Stop worrying about semicolons and macro syntax. Define your `UCLASS`, `USTRUCT`, and `UENUM` definitions using a native Unreal `UEdGraph` interface.
*   **Drag & Drop Type Safety:** Drag existing CodeForge assets onto property nodes to automatically set types and `#include` paths.
*   **Automatic Prefixes:** Never forget an `A`, `U`, or `F` prefix again; the tool handles Unreal's naming conventions for you.

### 2. Zero-Dependency Generation
CodeForge features a bespoke, deterministic C++ template engine.
*   **Offline First:** 100% local processing. No internet connection or AI API credits required.
*   **UBT Compliant:** Generates clean, human-readable `.h` and `.cpp` files that follow Epic Games' architectural standards.

### 3. Real-Time Validation
The graph UI provides immediate feedback on your design.
*   **Rule Enforcement:** Detects duplicate names, invalid C++ keywords, and incompatible `UPROPERTY` specifiers (e.g., `BlueprintReadWrite` vs `BlueprintReadOnly`) instantly.
*   **Auto-Fixes:** Common issues (like missing `b` prefixes on booleans) can be resolved with a single click directly on the node.

### 4. Smart Code Injection
Inject raw C++ logic directly into `FunctionBody` and `ConstructorBody` fields in the Details Panel to generate fully functional gameplay logic without leaving the editor.

---

## ?? Visual Workflow

| **Action** | **Description** |
| :--- | :--- |
| **Model** | Connect a **Class Node** to multiple **Property** and **Function Nodes**. |
| **Configure** | Use the checkbox-driven UI to set specifiers like `EditAnywhere` or `Replicated`. |
| **Preview** | View the live-generated C++ code in the side-by-side **Preview Tab**. |
| **Forge** | Hit **Generate** to write files to your project and trigger **Live Coding**. |

---

## ?? Installation

1.  Clone this repository into your project's `Plugins` folder:
    ```bash
    cd YourProject/Plugins
    git clone https://github.com/gregorik/CodeForge CodeForge
    ```
2.  Restart the Unreal Editor.
3.  Right-click in the **Content Browser** > **Misc** > **CodeForge Blueprint** to create your first visual C++ model.

---

## ?? Pro vs. Core

CodeForge Core provides the essential foundation for visual C++ development. For professional teams and complex multiplayer projects, consider **CodeForge Pro** on the Fab Marketplace.

| Feature | Core (OSS) | Pro (Fab) |
| :--- | :---: | :---: |
| **UCLASS / USTRUCT / UENUM** | ✅ | ✅ |
| **Live Code Preview** | ✅ | ✅ |
| **Validation & Auto-Fix** | ✅ | ✅ |
| **Network RPCs (Server/Client/Multicast)** | ❌ | ✅ |
| **Dynamic Multicast Delegates** | ❌ | ✅ |
| **UINTERFACE Support** | ❌ | ✅ |
| **Advanced GA Auto-Layout** | ❌ | ✅ |
| **RPG Starter Kit (Examples)** | ❌ | ✅ |

---

## ?? License

Distributed under the MIT License. See `LICENSE` for more information.

---

## ?? Contributing

Contributions are welcome! Whether it's reporting bugs, suggesting new core templates, or improving documentation, please feel free to open an issue or submit a pull request.

**Crafted with ?? by GregOrigin.**
