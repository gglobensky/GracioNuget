import { Gracio } from './gracio.js';

// This loads the compiled C++ addon. 
// The path corresponds to where node-gyp puts the output binary.
let native;
try {
    native = await import('../build/Release/gracio_native.node');
} catch (e) {
    console.warn('[Gracio Native] Could not load native bindings. Falling back to Pure TS implementation.');
    native = null;
}

export const NativeBridge = {
    createFromInts: (n: bigint, d: bigint) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.fromInt(n, d);
    },
    createFromFloat: (val: number) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.fromFloat(val);
    },
    add: (a: any, b: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.add(a, b);
    },
    subtract: (a: any, b: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.subtract(a, b);
    },
    multiply: (a: any, b: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.multiply(a, b);
    },
    divide: (a: any, b: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.divide(a, b);
    },
    pow: (a: any, exp: bigint) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.pow(a, exp);
    },
    root: (index: number, val: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.root(index, val);
    },
    approximate: (val: any, digits: number) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.approximate(val, digits);
    },
    toDouble: (val: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.toDouble(val);
    },
    toString: (val: any) => {
        if (!native) throw new Error("Native bindings not loaded");
        return native.Gracio.toString(val);
    }
};
