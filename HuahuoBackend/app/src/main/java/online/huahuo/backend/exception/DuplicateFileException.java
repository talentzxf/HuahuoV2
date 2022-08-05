package online.huahuo.backend.exception;

public class DuplicateFileException extends HuahuoServiceException{
    public DuplicateFileException(String fileName){
        super("File:" + fileName + " already exists!");
    }
}
