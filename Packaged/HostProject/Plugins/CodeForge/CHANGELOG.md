# CodeForge Changelog

## [0.2.0] - 2026-05-07

### Added
- **Code Preview Panel**: Added a real-time preview panel showing generated `.h` and `.cpp` code alongside the node graph.
- **Asset Creation Wizard**: Introduced a new visual setup wizard for asset creation, featuring archetype selection cards for Actors, Components, Structs, Enums, and Interfaces.
- **Real-Time Validation**: Validation errors and warnings are now visualized directly on the relevant graph nodes.
- **Custom Graph Nodes**: Added `SGraphNode_CodeForge` to provide color-coded node headers, error badges, and improved pin layouts.
- **Undo/Redo Transactions**: Full transaction support added for all graph operations, ensuring safe undo/redo capabilities.
- **IncludePath Support**: Added an `IncludePath` field to Property definitions for automatically handling custom type includes.
- **Two-Way Synchronization**: Implemented `SyncFromBlueprint` to keep graph nodes strictly synchronized with the underlying data asset.
- **Plugin Settings**: Added a global CodeForge settings page (in Project Settings) to configure brand colors, default prefixes, and Live Coding behaviors.
- **Marketplace Metadata**: Configured plugin for distribution and added Fab marketplace metadata.
- **Motion Canvas Project**: Scaffolded a demo-video project using Motion Canvas for marketing and showcases.

### Fixed
- **Graph Tab Visibility**: Fixed an issue where the Editor's Graph tab failed to display due to an incorrect layout role.
- **Wizard Interactivity**: Fixed the archetype selection cards in the wizard by switching from unresponsive buttons to visually distinct toggle checkboxes.
- **Editor Stability**: Hardened editor stability by improving null-safety, handling missing graph commands, and refining output path resolution.
- **UE5.7 Build**: Addressed various UE5.7 specific build and compiler errors across the plugin.

### Documentation
- **Updated User Manual & FAQ**: Extensively updated `index.html` and `faq.html` to cover the new features (Preview Panel, Real-time Validation, Settings, Undo/Redo) and bumped documentation version to v0.2.0.