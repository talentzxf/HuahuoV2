interface ChainCallback<T> {
    (param:T):boolean;
}

// T is the call back param type
class ResponsibleChainHandler<T>{
    private next: ResponsibleChainHandler<T> | null = null
    private readonly callBack: ChainCallback<T> | null

    public constructor(inCallBack: ChainCallback<T>| null){
        this.callBack = inCallBack
    }

    public SetNext(inNext:ResponsibleChainHandler<T>| null){
        this.next = inNext
    }

    public GetNext(): ResponsibleChainHandler<T> | null{
        return this.next
    }

    /**
     * Return true -- break the execution, didn't execute all remaining operations in the chain
     * Return false -- continue the chain execution
     * @param param
     */
    public execute(param: T): boolean{
        if(this.callBack != null && this.callBack(param)){
            return true
        }

        if(this.next != null){
            return this.next.execute(param)
        }

        return false
    }
}

class ResponsibleChain<T>{
    private handlerChain: ResponsibleChainHandler<T> | null = null

    public AddFront(frontCallBack: ChainCallback<T>):void{
        const wrappedHandler = new ResponsibleChainHandler(frontCallBack)
        if(this.handlerChain != null){
            wrappedHandler.SetNext(this.handlerChain);
        }

        this.handlerChain = wrappedHandler
    }

    public AddBack(lastCallBack: ChainCallback<T>):void{
        const wrappedHandler = new ResponsibleChainHandler(lastCallBack)
        if(this.handlerChain == null){
            this.handlerChain = wrappedHandler;
        }else if(this.handlerChain.GetNext() == null){
            this.handlerChain.SetNext(wrappedHandler)
        }else{ // Recursively find the last handler
            let curHandler = this.handlerChain
            let nextHandler = this.handlerChain.GetNext()

            while(nextHandler != null){
                curHandler = nextHandler
                nextHandler = nextHandler.GetNext()
            }

            curHandler.SetNext(wrappedHandler)
        }
    }

    public execute(param: T): boolean{
        if(this.handlerChain != null){
            return this.handlerChain.execute(param)
        }

        return false
    }
}

export {ResponsibleChain, ChainCallback}