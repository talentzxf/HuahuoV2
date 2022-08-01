package online.huahuo.backend.security;

import lombok.Getter;
import lombok.RequiredArgsConstructor;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.context.properties.ConstructorBinding;

import java.time.Duration;

@ConstructorBinding
@ConfigurationProperties(prefix = "huahuo.jwt")
@Getter
@RequiredArgsConstructor
public class SecurityProperties implements InitializingBean {
    /**
     * Amount of hashing iterations, where formula is 2^passwordStrength iterations
     */
    private final int passwordStrength;
    /**
     * Secret used to generate and verify JWT tokens
     */
    private final String tokenSecret;
    /**
     * Name of the token issuer
     */
    private final String tokenIssuer;

    private final int tokenExpirationHours;
    /**
     * Duration after which a token will expire
     */
    private Duration tokenExpiration;

    @Override
    public void afterPropertiesSet() throws Exception {
        tokenExpiration = Duration.ofHours(tokenExpirationHours);
    }
}

