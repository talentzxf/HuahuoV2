package online.huahuo.backend.exception;

public class BadTokenException extends HuahuoServiceException {
    private static final long serialVersionUID = 158136221282852553L;

    @Override
    public String getMessage() {
        return "Token is invalid or expired";
    }
}
