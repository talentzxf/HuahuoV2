export class IDGenerator{
    private curId = 0;
    private static inst:IDGenerator = null;

    private constructor() {
    }

    private static Inst():IDGenerator{
        if(IDGenerator.inst == null){
            IDGenerator.inst = new IDGenerator()
        }
        return IDGenerator.inst
    }

    getId():Number{
        return this.curId++;
    }
}