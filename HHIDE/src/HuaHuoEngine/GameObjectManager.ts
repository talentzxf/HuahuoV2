import {EngineAPI} from "../EngineAPI";

class GameObjectManager{
    private static _instance:GameObjectManager
    private constructor() {
    }

    public static getInstance():GameObjectManager{
        if(GameObjectManager._instance == null){
            GameObjectManager._instance = new GameObjectManager()
        }
        return GameObjectManager._instance;
    }

    createGameObject(parentObject?:any){
        EngineAPI.getInstance().CreateGameObject("Empty");
    }
}

export {GameObjectManager}