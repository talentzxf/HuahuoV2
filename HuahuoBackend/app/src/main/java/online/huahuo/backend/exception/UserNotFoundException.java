package online.huahuo.backend.exception;

public class UserNotFoundException extends HuahuoServiceException {
    public UserNotFoundException(Long id){
        super("Could not find user:" + id);
    }

    public UserNotFoundException(String username){
        super("Cound not find user:" + username);
    }
}
