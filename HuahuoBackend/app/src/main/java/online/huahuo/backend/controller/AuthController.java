package online.huahuo.backend.controller;

import lombok.Data;
import online.huahuo.backend.security.JwtTokenUtil;
import online.huahuo.backend.security.JwtUserDetailsService;
import online.huahuo.backend.utils.Utils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.security.authentication.DisabledException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.web.bind.annotation.*;

import java.io.Serializable;

@Data
class JwtRequest implements Serializable{

    private static final long serialVersionUID = 3937684001396083232L;

    private String username;
    private String password;
}

@Data
class JwtResponse implements Serializable{

    private static final long serialVersionUID = -7563245793120953012L;

    private final String jwttoken;
}

@RestController
@CrossOrigin
public class AuthController {
    @Autowired
    private AuthenticationManager authenticationManager;

    @Autowired
    private JwtTokenUtil jwtTokenUtil;

    @Autowired
    private JwtUserDetailsService userDetailsService;

    @RequestMapping(value = "authenticate", method = RequestMethod.POST)
    public ResponseEntity<?> generateAuthToken(@RequestBody JwtRequest authRequest) throws Exception {
        authenticate(authRequest.getUsername(), authRequest.getPassword());

        final UserDetails userDetails = userDetailsService.loadUserByUsername(authRequest.getUsername());

        final String token = jwtTokenUtil.generateToken(userDetails);

        return ResponseEntity.ok(new JwtResponse(token));
    }

    private void authenticate(String username, String password) throws Exception{
        try{
            String pwdHash = Utils.hashString(password);
            authenticationManager.authenticate(new UsernamePasswordAuthenticationToken(username, pwdHash));
        }catch (DisabledException e){
            throw new Exception("USER_DISABLED", e);
        }catch (BadCredentialsException e){
            throw new Exception("Invalid credential", e);
        }
    }
}
