package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.exception.UserNotFoundException;
import online.huahuo.backend.utils.Utils;
import org.springframework.security.access.annotation.Secured;
import org.springframework.web.bind.annotation.*;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.List;

@RestController
@AllArgsConstructor
public class UserController {
    private final UserRepository userRepository;

    @Secured("ADMIN")
    @GetMapping("/users")
    List<UserDB> all(){
        return (List<UserDB>) userRepository.findAll();
    }

    @GetMapping("/users/{id}")
    UserDB one(@PathVariable Long id){
        return userRepository.findById(id).orElseThrow( ()-> new UserNotFoundException(id));
    }

    @Secured("ADMIN")
    @PostMapping("/users")
    UserDB newUser(@RequestBody UserDB userDB) throws NoSuchAlgorithmException {
        String rawPassword = userDB.getPassword();
        String hashedPassword = Utils.hashString(rawPassword);
        userDB.setPassword(hashedPassword);
        return userRepository.save(userDB);
    }
}
