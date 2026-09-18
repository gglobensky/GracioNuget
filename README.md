# 📐 Gracio (NuGet Edition)

A high-performance, arbitrary-precision rational arithmetic library for .NET and C++.
Ported from the original Node.js implementation with extreme optimizations for bare-metal memory management.

## 🚀 Key Improvements in this Version
- **Small Integer Optimization (SIO)**: Common ratios are stored on the stack to avoid heap allocations.
- **Native Performance**: Core arithmetic is implemented in C++ using binary GCD and continued fraction approximations.
- **Zero GC Pressure**: The mutable API allows you to perform millions of operations without triggering .NET Garbage Collection.

## 🛠 Usage (C#)

```csharp
using Gracio;

// Create from integers
var a = new Ratio(10, 1); 
var b = new Ratio(1, 3);

// Mutable arithmetic for maximum speed
a.Add(b); // 'a' is now 31/3
a.Multiply(new Ratio(2, 1)); // 'a' is now 62/3

Console.WriteLine(a.ToString()); // "62/3"
Console.WriteLine(a.ToDouble()); // 20.666...
```

## 📦 Build & Publish
1. **Build Native**: Compile `native/` as a DLL (`gracio.dll`).
2. **Build Managed**: Run `dotnet build managed/Gracio.csproj`.
3. **Pack**: Use `dotnet pack` with the native binaries placed in `runtimes/win-x64/native/`.
