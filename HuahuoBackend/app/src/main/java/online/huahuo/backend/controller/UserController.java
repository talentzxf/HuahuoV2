package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRole;
import online.huahuo.backend.db.UserService;
import online.huahuo.backend.exception.UserNotFoundException;
import online.huahuo.backend.utils.Utils;
import org.apache.commons.lang3.RandomStringUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.annotation.Secured;
import org.springframework.web.bind.annotation.*;

import java.security.NoSuchAlgorithmException;
import java.util.List;

@RestController
@RequiredArgsConstructor
public class UserController {
    private final String passwordMask = "XXXXXXXXXX";

    private final UserService userService;

    @Value("${huahuo.anonymous.usernameLength}")
    private int anonymousUserNameLength;

    @Value("${huahuo.anonymous.passwordLength}")
    private int anonymousPwdLength;

    @Secured("ADMIN")
    @GetMapping("/users")
    List<UserDB> all(){
        return (List<UserDB>) userService.findAll();
    }

    @GetMapping("/users/{id}")
    UserDB one(@PathVariable Long id){
        return userService.findById(id).orElseThrow( ()-> new UserNotFoundException(id));
    }

//    // TODO: Disable cross origin in PROD!
//    @CrossOrigin(origins = "http://127.0.0.1:8989")
    @PostMapping("/users")
    ResponseEntity<UserDB> newUser(@RequestHeader(required = false) Boolean isAnonymous, @RequestBody(required = false) UserDB user) throws NoSuchAlgorithmException {
        if(isAnonymous == null && user == null){
            return new ResponseEntity<>(null, HttpStatus.BAD_REQUEST);
        }

        if(isAnonymous == null || !isAnonymous){
            String rawPassword = user.getPassword();
            String hashedPassword = Utils.hashString(rawPassword);
            user.setPassword(hashedPassword);

            UserDB usrDB = userService.save(user);
            usrDB.setPassword(passwordMask);
            return new ResponseEntity<UserDB>(usrDB, HttpStatus.OK);
        }else{
            String username = RandomStringUtils.random(anonymousUserNameLength, true, true);

            while(userService.findByUsername(username) != null){
                username = RandomStringUtils.random(anonymousUserNameLength, true, true);
            }

            String pwd = RandomStringUtils.random(anonymousPwdLength, true, true);
            UserDB usr = userService.createUser(username, pwd, UserRole.ANONYMOUS);

            // Set the pwd back for anonymous users.
            usr.setPassword(pwd);

            if(null == usr){
                return new ResponseEntity<>(null, HttpStatus.SERVICE_UNAVAILABLE);
            }else{
                return new ResponseEntity<>(usr, HttpStatus.OK);
            }
        }
    }
}
