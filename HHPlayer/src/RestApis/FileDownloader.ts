import huahuoProperties from "../hhplayer.properties";
import axios, {AxiosError} from "axios";

class FileDownloader{
    downloadFile(projectId:string, onSuccess: Function, onFailed: Function){
        let absoluteUrl = huahuoProperties["huahuo.backend.url"] + "/projects/" + projectId
        axios.get(absoluteUrl, {responseType: 'blob'}).then(
            response=>{
                let fileName = response.headers["x-suggested-filename"].split("filename=")[1];
                onSuccess(response.data, fileName)
            }
        ).catch((reason: AxiosError)=>{
            onFailed(reason.status, reason.message)
        })
    }
}

let fileDownloader = new FileDownloader()

export {fileDownloader}
