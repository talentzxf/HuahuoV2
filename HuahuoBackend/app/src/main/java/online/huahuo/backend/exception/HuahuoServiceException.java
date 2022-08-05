package online.huahuo.backend.exception;

public abstract class HuahuoServiceException extends RuntimeException{
    public HuahuoServiceException() {
    }

    public HuahuoServiceException(String message) {
        super(message);
    }

    public HuahuoServiceException(String message, Throwable cause) {
        super(message, cause);
    }
}
