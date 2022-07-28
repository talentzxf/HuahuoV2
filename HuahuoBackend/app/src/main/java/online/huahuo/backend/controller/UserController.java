package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.exception.UserNotFoundException;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@AllArgsConstructor
public class UserController {
    private final UserRepository userRepository;

    @GetMapping("/users")
    List<UserDB> all(){
        return (List<UserDB>) userRepository.findAll();
    }

    @GetMapping("/users/{id}")
    UserDB one(@PathVariable Long id){
        return userRepository.findById(id).orElseThrow( ()-> new UserNotFoundException(id));
    }

    @PostMapping("/users")
    UserDB newUser(@RequestBody UserDB userDB){
        return userRepository.save(userDB);
    }
}
