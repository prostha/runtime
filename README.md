# Prostha Runtime


This agnostic simulation runtime is built in C++23 and strictly follows a simple, fast, and secure philosophy.
These coherent rules transformed what started as a fun and exciting project into a daunting challenge, reminiscent of the Grayskull fortress.
It is designed this way to eliminate dependencies and avoid sloppy code in the future.
The engine utilizes an index-based data model, historically referred to as archetype architecture, which has proven to be one of the most efficient management algorithms, achieving O(1) complexity.
It offers a scripting sandbox arena that combines script execution with a dedicated allocator for specialized memory management.
It natively supports Vulkan, DirectX 12, and Metal for graphics rendering. 
This open-source engine is optimized and stripped down to the absolute minimum footprint, designed to seamlessly integrate into other projects. It delivers robust simulation capabilities without sacrificing performance.


## Contributing

Contributions, Contributions? It is really needed, support is really appreciated. 
Whether you are fixing a bug in the dense page allocator, improving graphics compute dispatches, or expanding our Lua mapping capabilities, your help is welcome.

To get started:

1. Review our [Contributing Guidelines](https://github.com/prostha/CONTRIBUTIONS.md) for local development workflows and coding standards (C++23 style conventions).
2. Look at the open issues or open a discussion thread to propose major changes before starting work.
3. Submit a Pull Request targeting the development branch. Ensure your structural changes adhere to the strict design constraints.

## License

This project is open-source software, licensed under the terms of the **GNU Lesser General Public License v3.0 (LGPL-3.0)**.

You can find the full text of the license alongside explicit terms and permissions in the accompanying [LICENSE](LICENSE.md) file. This allows you to bundle, extend, or integrate the runtime directly into your simulation tools and graphic sandboxes while ensuring core ecosystem improvements remain open.