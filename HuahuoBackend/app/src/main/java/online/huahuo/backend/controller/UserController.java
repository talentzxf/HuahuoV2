package online.huahuo.backend.controller;

import io.swagger.v3.oas.annotations.security.SecurityRequirements;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRole;
import online.huahuo.backend.db.UserService;
import online.huahuo.backend.exception.UserNotFoundException;
import online.huahuo.backend.security.UserPasswordEncoder;
import org.apache.commons.lang3.RandomStringUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.annotation.Secured;
import org.springframework.web.bind.annotation.*;

import java.security.NoSuchAlgorithmException;
import java.util.Date;
import java.util.List;

@RestController
@RequiredArgsConstructor
public class UserController {
    private final String passwordMask = "XXXXXXXXXX";

    private final UserService userService;

    private final UserPasswordEncoder userPasswordEncoder;

    @Value("${huahuo.anonymous.usernameLength}")
    private int anonymousUserNameLength;

    @Value("${huahuo.anonymous.passwordLength}")
    private int anonymousPwdLength;

    @Secured("ADMIN")
    @GetMapping("/users")
    List<UserDB> all() {
        return (List<UserDB>) userService.findAll();
    }

    // TODO: Avoid DDOS.
    @GetMapping("/users/exist")
    ResponseEntity<UserExistResponse> exist(@RequestParam String username) {
        UserDB userDB = userService.findByUsername(username);

        UserExistResponse response = new UserExistResponse();
        response.setUsername(username);

        if (userDB != null) {
            response.setExist(true);
        } else {
            response.setExist(false);
        }
        return new ResponseEntity<>(response, HttpStatus.OK);
    }

    @GetMapping("/users/{id}")
    UserDB one(@PathVariable Long id) {
        return userService.findById(id).orElseThrow(() -> new UserNotFoundException(id));
    }

    @SecurityRequirements()
    @PostMapping("/users")
    ResponseEntity<UserDB> createUser(@RequestHeader(required = false) Boolean isAnonymous, @RequestBody(required = false) UserDB user) throws NoSuchAlgorithmException {
        if (isAnonymous == null && user == null) {
            return new ResponseEntity<>(null, HttpStatus.BAD_REQUEST);
        }

        if (isAnonymous == null || !isAnonymous) {
            if (user.getUsername() == null || user.getPassword() == null) {
                return new ResponseEntity<>(null, HttpStatus.BAD_REQUEST);
            }

            String rawPassword = user.getPassword();
            String hashedPassword = userPasswordEncoder.encode(rawPassword);
            user.setPassword(hashedPassword);
            user.setCreateTime(new Date());
            user.setModifiedTime(new Date());
            user.setLastLoginTime(new Date());

            UserDB usrDB = userService.save(user);
            usrDB.setPassword(passwordMask);
            return new ResponseEntity<UserDB>(usrDB, HttpStatus.OK);
        } else {
            String username = RandomStringUtils.random(anonymousUserNameLength, true, true);

            while (userService.findByUsername(username) != null) {
                username = RandomStringUtils.random(anonymousUserNameLength, true, true);
            }

            String pwd = RandomStringUtils.random(anonymousPwdLength, true, true);
            UserDB usr = userService.createUser(username, pwd, UserRole.ANONYMOUS);

            // Set the pwd back for anonymous users.
            usr.setPassword(pwd);

            if (null == usr) {
                return new ResponseEntity<>(null, HttpStatus.SERVICE_UNAVAILABLE);
            } else {
                return new ResponseEntity<>(usr, HttpStatus.OK);
            }
        }
    }
}
