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
            public static extern void gracio_set_precision_limit(IntPtr g, uint limit);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_add(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_subtract(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_multiply(IntPtr a, IntPtr b);

            [DllImport("gracio_native", CallingConvention = CallingConvention.Cdecl)]
            public static extern void gracio_divide(IntPtr a, IntPtr b);

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

        // --- API ---
        public uint PrecisionLimit {
            get => 0; // Getter not implemented in C-API yet
            set => Native.gracio_set_precision_limit(_handle, value);
        }

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
            Native.gracio_divide(_handle, other._handle);
            return this;
        }

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
