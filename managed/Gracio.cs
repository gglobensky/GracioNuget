using System;
using System.Runtime.InteropServices;

namespace Gracio {
    public class Ratio : IDisposable {
        private IntPtr _handle;
        private bool _disposed = false;

        // --- Native Imports ---
        private static class Native {
            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr gracio_create_int(long numerator, long denominator);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr gracio_create_float(double value);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_destroy(IntPtr g);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr gracio_clone(IntPtr g);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_set_precision_limit(IntPtr g, uint limit);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_add(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_subtract(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_multiply(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern int try_gracio_divide(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_power(IntPtr a, long exp);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr gracio_root(uint index, IntPtr value);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern double gracio_to_double(IntPtr g);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr gracio_to_string(IntPtr g);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_free_string(IntPtr s);
        }

        // --- Constructors ---
        public Ratio(long numerator, long denominator) {
            _handle = Native.gracio_create_int(numerator, denominator);
            if (_handle == IntPtr.Zero) throw new OutOfMemoryException("Failed to create native Gracio object.");
        }

        public Ratio(double value) {
            _handle = Native.gracio_create_float(value);
            if (_handle == IntPtr.Zero) throw new OutOfMemoryException("Failed to create native Gracio object.");
        }

        public static Ratio ParseNumber(string s) {
            if (s.Contains('.')) {
                return new Ratio(double.Parse(s));
            } else {
                return new Ratio(long.Parse(s), 1);
            }
        }

        // Internal constructor for cloning/internal use
        internal Ratio(IntPtr handle) {
            _handle = handle;
        }

        // --- Lifecycle & Utilities ---
        public Ratio Clone() {
            CheckDisposed();
            IntPtr clonedHandle = Native.gracio_clone(_handle);
            if (clonedHandle == IntPtr.Zero) throw new OutOfMemoryException("Failed to clone native Gracio object.");
            return new Ratio(clonedHandle);
        }

        public uint PrecisionLimit {
            get => 0; // Getter not implemented in C-API yet
            set => Native.gracio_set_precision_limit(_handle, value);
        }

        // --- Mutable API (In-place) ---
        public Ratio Add(Ratio other) {
            CheckDisposed();
            Native.gracio_add(_handle, other._handle);
            return this;
        }

        public Ratio Subtract(Ratio other) {
            CheckDisposed();
            Native.gracio_subtract(_handle, other._handle);
            return this;
        }

        public Ratio Multiply(Ratio other) {
            CheckDisposed();
            Native.gracio_multiply(_handle, other._handle);
            return this;
        }

        public Ratio Divide(Ratio other) {
            CheckDisposed();
            int result = Native.try_gracio_divide(_handle, other._handle);
            if (result == 1) throw new DivideByZeroException("Cannot divide by zero.");
            if (result == -1) throw new ArgumentException("Invalid native handles provided.");
            return this;
        }

        public Ratio Power(long exp) {
            CheckDisposed();
            Native.gracio_power(_handle, exp);
            return this;
        }

        // --- Operator Overloads (Immutable-style) ---
        public static Ratio operator +(Ratio a, Ratio b) {
            return a.Clone().Add(b);
        }

        public static Ratio operator -(Ratio a, Ratio b) {
            return a.Clone().Subtract(b);
        }

        public static Ratio operator *(Ratio a, Ratio b) {
            return a.Clone().Multiply(b);
        }

        public static Ratio operator /(Ratio a, Ratio b) {
            return a.Clone().Divide(b);
        }

        public static Ratio operator ^(Ratio a, long exp) {
            return a.Clone().Power(exp);
        }

        public static IntPtr NativeRoot(uint index, Ratio value) {
            return Native.gracio_root(index, value._handle);
        }

        public static Ratio Evaluate(string expression) {
            var lexer = new Lexer();
            var tokens = lexer.Tokenize(expression);
            var parser = new Parser(tokens);
            return parser.Parse();
        }

        // --- Output ---
        public double ToDouble() {
            CheckDisposed();
            return Native.gracio_to_double(_handle);
        }

        public override string ToString() {
            CheckDisposed();
            IntPtr strPtr = Native.gracio_to_string(_handle);
            try {
                return Marshal.PtrToStringAnsi(strPtr) ?? "null";
            } finally {
                Native.gracio_free_string(strPtr);
            }
        }

        // --- Lifecycle ---
        private void CheckDisposed() {
            if (_disposed) throw new ObjectDisposedException(nameof(Ratio));
        }

        public void Dispose() {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing) {
            if (!_disposed) {
                Native.gracio_destroy(_handle);
                _disposed = true;
            }
        }

        ~Ratio() {
            Dispose(false);
        }
    }
}
