package online.huahuo.backend.exception;

public class UserNotFoundException extends ServiceException{
    public UserNotFoundException(Long id){
        super("Could not find user:" + id);
    }
}
