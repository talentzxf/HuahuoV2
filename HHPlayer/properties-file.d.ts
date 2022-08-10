// properties-file.d.ts
declare module '*.properties' {
    const properties: { readonly [key: string]: string };
    export default properties;
}