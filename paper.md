---
title: "SRTOS: Simple deterministic real-time operating system for the ARM Cortex-M4/M4F"
tags:
  - C
  - embedded systems
  - operating system
  - kernel
  - ARM Cortex-M
authors:
  - name: Kirill McQuillin
    orcid: 0009-0009-7142-5990
    affiliation: 1

affiliations:
  - name: Innovation Academy, United States
    index: 1

date: 8 November 2025
bibliography: paper.bib
---

# Summary

SRTOS (Simple Real-Time Operating System) is a small preemptive RTOS for the ARM-Cortex M4/M4F processor designed for teaching and research, emphasizing safety, transparency, determinism, and ease-of-use. By design, SRTOS does not use dynamic memory allocation, enforces stack overflow checks and hard fault handling, and contains a minimal scheduler suitable for experimentation. The project provides extensive documentation, reproducible examples, and hardware-validated tests. SRTOS enables education and research on scheduling, timing analysis, fault handling, and deterministic behavior with a compact and readable codebase that is easy to extend.

# Statement of need

Research and education in real-time embedded systems often require an RTOS that is small, simple, and understandable, but also deterministic, capable, and memory-safe. While production RTOSes like FreeRTOS or Zephyr OS provide large, powerful codebases with complex features, they are often difficult to grasp conceptually or use in low-level experimentation, research, and testing.

These are the exact requirements and problems that SRTOS addresses by providing a safe codebase that is easy to understand, test, and modify for the ARM-Cortex M4/M4F processors. It features a preemptive round-robin scheduler, a purely static memory allocation scheme, stack overflow detection using canary values, stack usage reporting by stack watermarking, and hard fault handling. It is designed as a platform for teaching and research on task scheduling, timing analysis, memory-safe code, and fault handling. The minimal design and detailed documentation of SRTOS allows researchers and students to inspect every layer of the kernel and reproduce timing behavior on real hardware without having to worry about unnecessary complexities.

By combining core RTOS features like deterministic scheduling, safe memory usage, fault handling, and hardware reproducible testing with a minimal and readable codebase, SRTOS supports research projects, educational labs, and embedded-systems experiments that require verifiable and predictable real-time behavior.

# Acknowledgements

This work was inspired by the open-source communities behind production-grade real-time operating systems such as FreeRTOS and Zephyr OS.

# References
